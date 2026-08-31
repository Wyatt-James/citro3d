#include "internal.h"
#include <stdlib.h>
#include <c3d/base.h>
#include <c3d/effect.h>
#include <c3d/uniforms.h>
#include <c3d/profiler.h>
#include <c3d/gpucmd.h>
#include <c3d/renderqueue.h>

C3D_Context __C3D_Context;
static C3D_GpuCmdBuffer lastCmdBuffer;

static void defaultRestoreHook() {} // Stub

static void (*restoreHook) (void) = defaultRestoreHook;
static int currentBufCount; // This tracks global GPU state, so it doesn't belong in the context.
static aptHookCookie hookCookie;

__attribute__((weak)) void C3Di_LightEnvDirty(C3D_LightEnv* env)
{
	(void)env;
}

__attribute__((weak)) void C3Di_ProcTexUpdate(C3D_Context* ctx)
{
	(void)ctx;
}

__attribute__((weak)) void C3Di_ProcTexDirty(C3D_Context* ctx)
{
	(void)ctx;
}

static void C3Di_AptEventHook(APT_HookType hookType, C3D_UNUSED void* param)
{
	switch (hookType)
	{
		case APTHOOK_ONSUSPEND:
		{
			C3Di_RenderQueueWaitDone();
			C3Di_RenderQueueDisableVBlank();
			break;
		}
		case APTHOOK_ONRESTORE:
		{
			C3Di_RenderQueueEnableVBlank();
			C3D_ForceContextDirty();
			restoreHook();
			break;
		}
		default:
			break;
	}
}

bool C3Di_Init(size_t cmdBufSize, size_t gxQueueSize, bool doubleBuf)
{
	int i;
	size_t allocSize;
	u8 *cmdBuf;
	C3D_Context* ctx = C3Di_GetContext();

	if (ctx->flags & C3DiF_Active)
		return false;
	
	lastCmdBuffer = (C3D_GpuCmdBuffer) {NULL, 0, 0};

	cmdBufSize = (cmdBufSize + 0xF) &~ 0xF; // 0x10-byte align
	ctx->cmdBufSize = cmdBufSize/4;
	allocSize = doubleBuf ? cmdBufSize * 2 : cmdBufSize;
	cmdBuf = (u8*)linearAlloc(allocSize);
	if (!cmdBuf)
		return false;

	ctx->cmdBufs[0] = (u32*)cmdBuf;
	ctx->cmdBufs[1] = (u32*)(doubleBuf ? &cmdBuf[cmdBufSize] : NULL);
	ctx->cmdBufUsage = 0;

	// Allocate GX Queues
	for (i = 0; i < 2; i++)
	{
		ctx->gxQueues[i].maxEntries = gxQueueSize;
		ctx->gxQueues[i].entries = (gxCmdEntry_s*)malloc(gxQueueSize*sizeof(gxCmdEntry_s));
		if (!ctx->gxQueues[i].entries)
		{
			linearFree(ctx->cmdBufs[0]);
			ctx->cmdBufs[0] = NULL;
			ctx->cmdBufs[1] = NULL;
			if (ctx->gxQueues[0].entries)
				free(ctx->gxQueues[0].entries);
			ctx->gxQueues[0].entries = NULL;
			ctx->gxQueues[1].entries = NULL;
			return false;
		}
	}

	ctx->cmdBuf = ctx->cmdBufs[0];
	ctx->gxQueue = &ctx->gxQueues[0];

	ctx->flags = C3DiF_Active | C3DiF_TexEnvBuf | C3DiF_TexEnvAll | C3DiF_Effect | C3DiF_TexStatus | C3DiF_TexAll;
	if (doubleBuf)
		ctx->flags |= C3DiF_DoubleBuf;

	C3D_EffectInit(C3D_GetEffect());
	
	ctx->texConfig = BIT(12);
	ctx->texShadow = BIT(0);
	ctx->texEnvBuf = 0;
	ctx->texEnvBufClr = 0xFFFFFFFF;
	ctx->fogClr = 0;
	ctx->fogLut = NULL;

	for (i = 0; i < 3; i ++)
		ctx->tex[i] = NULL;

	for (i = 0; i < 6; i ++)
		C3D_TexEnvInit(&ctx->texEnv[i]);

	ctx->fixedAttribDirty = 0;
	ctx->fixedAttribEverDirty = 0;

	// init both gxQueues
	C3Di_RenderQueueInit(&ctx->gxQueues[1]);
	C3Di_RenderQueueInit(&ctx->gxQueues[0]);
	
	aptHook(&hookCookie, C3Di_AptEventHook, NULL);
	currentBufCount = 12;

	// Reset profiler functions
	C3D_ProfilerFunc(NULL);
	C3D_ProfilerCategoryClearAll(0);
	C3D_ProfilerCategoryEnableAll(false);

	// Forcefully send defaults to GPU
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	ctx->program = &(shaderProgram_s) {.geometryShader = (shaderInstance_s*) 1}; // Required for geoshader check
	C3D_UpdateContext();
	ctx->program = NULL;
	C3D_FrameEnd(0);
	C3Di_RenderQueueWaitDone();

	return true;
}

bool C3D_Init(size_t cmdBufSize)
{
	return C3Di_Init(cmdBufSize, 32 /* default gxQueueSize size */, false);
}

bool C3D_InitEx(size_t cmdBufSize, size_t gxQueueSize, bool doubleBuf)
{
	return C3Di_Init(cmdBufSize, gxQueueSize, doubleBuf);
}

void C3D_SetViewport(u32 x, u32 y, u32 w, u32 h)
{
	C3D_Context* ctx = C3Di_GetContext();
	ctx->flags |= C3DiF_Viewport | C3DiF_Scissor;
	C3D_SetViewportVal(&ctx->viewport, x, y, w, h);
	ctx->scissor.mode = GPU_SCISSOR_DISABLE;
}

void C3D_SetScissor(GPU_SCISSORMODE mode, u32 left, u32 top, u32 right, u32 bottom)
{
	C3D_Context* ctx = C3Di_GetContext();
	ctx->flags |= C3DiF_Scissor;
	C3D_SetScissorVal(&ctx->scissor, mode, left, top, right, bottom);
}

void C3D_UpdateContext(void)
{
	C3Di_GetProfiler()->log_slot_skipped = false;
	C3Di_Profile(C3D_ProfilerSlot_Misc);

	int i;
	C3D_Context* ctx = C3Di_GetContext();
	const u32 flags = ctx->flags;

	if (flags & C3DiF_FrameBuf)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_FrameBuf);
		C3D_SendFrameBuf(&ctx->fb, (flags & C3DiF_DrawUsed) != 0);
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_Viewport)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_Viewport);
		C3D_SendViewport(&ctx->viewport);
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_Scissor)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_Scissor);
		C3D_SendScissor(&ctx->scissor);
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_Program)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_Program);
		C3D_SendProgram(ctx->program, (flags & C3DiF_VshCode) != 0, (flags & C3DiF_GshCode) != 0);
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_AttrInfo)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_AttrInfo);
		C3D_SendAttrInfo(&ctx->attrInfo);
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_BufInfo)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_BufInfo);

		C3D_BufInfo* info = &ctx->bufInfo;
    	int bufsToWrite = currentBufCount > info->bufCount ? currentBufCount : info->bufCount; // Max
		C3D_SendBufInfo(info, bufsToWrite);
		currentBufCount = info->bufCount;

		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_Effect)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_Effect);
		C3D_SendEffect(&ctx->effect, C3DF_EFFECT_ALL);
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_TexAll)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_TexAll);

		u32 units = 0;
		for (i = 0; i < 3; i ++)
		{
			if (ctx->tex[i])
			{
				units |= BIT(i);
				if (flags & C3DiF_Tex(i))
					C3D_SendTex(i, ctx->tex[i]);
			}
		}

		// Enable texture units and clear texture cache
		ctx->texConfig &= ~7;
		ctx->texConfig |= units | BIT(16);

		C3Di_Profile_Exit_Block();
	}

	if (flags & (C3DiF_TexAll | C3DiF_TexStatus))
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_TexStatus);

		C3D_SendTexStatus(ctx->texConfig, ctx->texShadow);
		ctx->texConfig &= ~BIT(16);

		C3Di_Profile_Exit_Block();
	}

	if (flags & (C3DiF_ProcTex | C3DiF_ProcTexColorLut | C3DiF_ProcTexLutAll))
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_ProcTex);
		C3Di_ProcTexUpdate(ctx); // WYATT_TODO Re-reads & modifies ctx->flags!
		// C3D_SendProcTex()
		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_TexEnvBuf)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_TexEnvBuf);
		C3D_SendTexEnvBuf(ctx->texEnvBuf, ctx->texEnvBufClr, ctx->fogClr, C3DF_TEXENVBUF_ALL);
		C3Di_Profile_Exit_Block();
	}

	if ((flags & C3DiF_FogLut) && (ctx->texEnvBuf&7) != GPU_NO_FOG)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_FogLut);

		if (ctx->fogLut)
		{
			C3D_SendFogLut(ctx->fogLut);
		}

		C3Di_Profile_Exit_Block();
	}

	if ((ctx->texEnvBuf&7) == GPU_GAS)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_Gas);
		
		if (ctx->flags & C3DiF_Gas)
		{
			C3D_SendGas(&ctx->gasConfig);
			ctx->gasConfig.flags = 0;
		}
		
		if (ctx->flags & C3DiF_GasLut && ctx->gasLut)
		{
			C3D_SendGasLut(ctx->gasLut);
		}

		C3Di_Profile_Exit_Block();
	}

	if (flags & C3DiF_TexEnvAll)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_TexEnvAll);

		for (i = 0; i < 6; i ++)
		{
			if (flags & C3DiF_TexEnv(i))
			{
				C3D_SendTexEnv(i, &ctx->texEnv[i]);
			}
		}

		C3Di_Profile_Exit_Block();
	}

	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_LightEnv);
		C3D_LightEnv* env = ctx->lightEnv;

		if (flags & C3DiF_LightEnv)
		{
			C3D_SendLightEnvEnable(env);
		}

		if (env)
		{
			C3D_SendLightEnv(env);
		}

		C3Di_Profile_Exit_Block();
	}

	if (ctx->fixedAttribDirty)
	{
		C3Di_Profile_Enter_Block(C3D_ProfilerSlot_FixedAttribDirty);

		for (i = 0; i < 12; i ++)
		{
			if (!(ctx->fixedAttribDirty & BIT(i))) continue;
			C3D_SendFixedAttrib(&ctx->fixedAttribs[i], i);
		}
		ctx->fixedAttribDirty = 0;

		C3Di_Profile_Exit_Block();
	}

	// We could optimize this, but we'd have to 
	ctx->flags &= 
		~(C3DiF_FrameBuf  |
		  C3DiF_DrawUsed  |
		  C3DiF_Viewport  |
		  C3DiF_Scissor   |
		  C3DiF_Program   |
		  C3DiF_VshCode   |
		  C3DiF_GshCode   |
		  C3DiF_AttrInfo  |
		  C3DiF_BufInfo   |
		  C3DiF_Effect    |
		  C3DiF_TexAll    |
		  C3DiF_TexStatus |
		  C3DiF_TexEnvBuf |
		  C3DiF_FogLut    |
		  C3DiF_TexEnvAll |
		  C3DiF_Gas       |
		  C3DiF_GasLut    |
		  C3DiF_LightEnv);

	C3Di_Profile_Enter_Block(C3D_ProfilerSlot_UpdateUniforms);
	C3D_UpdateUniforms(GPU_VERTEX_SHADER);
	if (ctx->program->geometryShader != NULL)
		C3D_UpdateUniforms(GPU_GEOMETRY_SHADER);
	C3Di_Profile_Exit_Block();
}

bool C3Di_SplitFrame(u32** pBuf, u32* pOffset)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!gpuCmdBufOffset)
		return false; // Nothing was drawn

	if (ctx->flags & C3DiF_DrawUsed)
	{
		ctx->flags &= ~C3DiF_DrawUsed;
		GPUCMD_AddWrite(GPUREG_FRAMEBUFFER_FLUSH, 1);
		GPUCMD_AddWrite(GPUREG_FRAMEBUFFER_INVALIDATE, 1);
		GPUCMD_AddWrite(GPUREG_EARLYDEPTH_CLEAR, 1);
	}

	lastCmdBuffer.size = gpuCmdBufSize;

	GPUCMD_Split(pBuf, pOffset);

	lastCmdBuffer.buf = *pBuf;
	lastCmdBuffer.offset = *pOffset;

	u32 totalCmdBufSize = *pBuf + *pOffset - ctx->cmdBuf;
	ctx->cmdBufUsage = (float)totalCmdBufSize / ctx->cmdBufSize;
	return true;
}

float C3D_GetCmdBufUsage(void)
{
	return C3Di_GetContext()->cmdBufUsage;
}

void C3D_Fini(void)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	aptUnhook(&hookCookie);
	C3Di_RenderQueueExit();
	free(ctx->gxQueues[0].entries);
	free(ctx->gxQueues[1].entries);
	linearFree(ctx->cmdBuf);
	ctx->flags = 0;
}

void C3D_BindProgram(shaderProgram_s* program)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	shaderProgram_s* oldProg = ctx->program;
	shaderInstance_s* newGsh = program->geometryShader;
	if (oldProg != program)
	{
		ctx->program = program;
		ctx->flags |= C3DiF_Program | C3DiF_AttrInfo;

		if (!oldProg)
			ctx->flags |= C3DiF_VshCode | C3DiF_GshCode;
		else
		{
			shaderInstance_s* oldGsh = oldProg->geometryShader;

			DVLP_s* oldProgV = oldProg->vertexShader->dvle->dvlp;
			DVLP_s* oldProgG = oldGsh ? oldGsh->dvle->dvlp : oldProgV;

			DVLP_s* newProgV = program->vertexShader->dvle->dvlp;
			DVLP_s* newProgG = newGsh ? newGsh->dvle->dvlp : newProgV;

			if (oldProgV != newProgV || (!newGsh && oldProgG != newProgG))
				ctx->flags |= C3DiF_VshCode;
			if (oldProgG != newProgG || (newProgG==oldProgV && newProgG->codeSize >= 512))
				ctx->flags |= C3DiF_GshCode;
		}
	}

	C3Di_LoadShaderUniforms(program->vertexShader);
	if (newGsh)
		C3Di_LoadShaderUniforms(newGsh);
	else
		C3Di_ClearShaderUniforms(GPU_GEOMETRY_SHADER);
}

C3D_FVec* C3D_FixedAttribGetWritePtr(int id)
{
	if (id < 0 || id >= 12)
		return NULL;

	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return NULL;

	ctx->fixedAttribDirty     |= BIT(id);
	ctx->fixedAttribEverDirty |= BIT(id);
	return &ctx->fixedAttribs[id];
}

void C3D_NotifyDrawUsed(void)
{
	C3Di_GetContext()->flags |= C3DiF_DrawUsed;
}

void C3D_ForceContextDirty(void)
{
	C3D_Context* ctx = C3Di_GetContext();
	
	ctx->flags |= C3DiF_AttrInfo | C3DiF_BufInfo | C3DiF_Effect | C3DiF_FrameBuf
		| C3DiF_Viewport | C3DiF_Scissor | C3DiF_Program | C3DiF_VshCode | C3DiF_GshCode
		| C3DiF_TexAll | C3DiF_TexEnvBuf | C3DiF_TexEnvAll | C3DiF_LightEnv | C3DiF_Gas;

	C3Di_DirtyUniforms(GPU_VERTEX_SHADER);
	C3Di_DirtyUniforms(GPU_GEOMETRY_SHADER);

	ctx->fixedAttribDirty |= ctx->fixedAttribEverDirty;
	ctx->gasConfig.flags |= C3DiG_BeginAcc | C3DiG_AccStage | C3DiG_RenderStage;

	C3D_LightEnv* env = ctx->lightEnv;
	if (ctx->fogLut)
		ctx->flags |= C3DiF_FogLut;
	if (ctx->gasLut)
		ctx->flags |= C3DiF_GasLut;
	if (env)
		C3Di_LightEnvDirty(env);
	C3Di_ProcTexDirty(ctx);
	
	currentBufCount = 12;
}

void C3D_SetRestoreHook(void (*hook) (void))
{
	if (hook != NULL)
		restoreHook = hook;
	else
		restoreHook = defaultRestoreHook;
}

C3D_GpuCmdBuffer C3D_GetLastGpuCmdBuffer(void)
{
	return lastCmdBuffer;
}
