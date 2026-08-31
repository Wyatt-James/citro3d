#pragma once
#include <stdbool.h>
#include <c3d/base.h>
#include <c3d/attribs.h>
#include <c3d/buffers.h>
#include <c3d/proctex.h>
#include <c3d/light.h>
#include <c3d/framebuffer.h>
#include <c3d/texenv.h>
#include <c3d/fog.h>
#include <c3d/effect.h>

#include "internal_profiler.h"

#define C3D_UNUSED __attribute__((unused))
#define C3D_ASSUME if (!(cond)) __builtin_unreachable()
#define C3D_ALIGNED(n_) __attribute__((aligned(n_)))

typedef struct
{
	gxCmdQueue_s gxQueues[2];
	gxCmdQueue_s* gxQueue;
	u32 curCmdBufIndex;
	u32* cmdBufs[2];
	u32* cmdBuf;
	size_t cmdBufSize;
	float cmdBufUsage;

	u32 flags;
	shaderProgram_s* program;

	C3D_AttrInfo attrInfo;
	C3D_BufInfo bufInfo;
	C3D_Effect effect;
	C3D_LightEnv* lightEnv;

	u32 texConfig;
	u32 texShadow;
	C3D_Tex* tex[3];
	C3D_TexEnv texEnv[6];

	u32 texEnvBuf, texEnvBufClr;
	u32 fogClr;
	C3D_FogLut* fogLut;
	
	C3D_GasConfig gasConfig;
	C3D_GasLut* gasLut;

	C3D_ProcTex* procTex;
	C3D_ProcTexLut* procTexLut[3];
	C3D_ProcTexColorLut* procTexColorLut;

	C3D_FrameBuf fb;
	C3D_Viewport viewport;
	C3D_Scissor scissor;

	u16 fixedAttribDirty, fixedAttribEverDirty;
	C3D_FVec fixedAttribs[12];
} C3D_Context;

enum
{
	C3DiF_Active           = BIT(0),
	C3DiF_DrawUsed         = BIT(1),
	C3DiF_AttrInfo         = BIT(2),
	C3DiF_BufInfo          = BIT(3),
	C3DiF_Effect           = BIT(4),
	C3DiF_FrameBuf         = BIT(5),
	C3DiF_Viewport         = BIT(6),
	C3DiF_Scissor          = BIT(7),
	C3DiF_Program          = BIT(8),
	C3DiF_TexEnvBuf        = BIT(9),
	C3DiF_LightEnv         = BIT(10),
	C3DiF_VshCode          = BIT(11),
	C3DiF_GshCode          = BIT(12),
	C3DiF_DoubleBuf        = BIT(13),
	C3DiF_Gas              = BIT(14),
	C3DiF_GasLut           = BIT(15),
	C3DiF_FogLut           = BIT(16),
	C3DiF_ProcTex          = BIT(17),
	C3DiF_ProcTexColorLut  = BIT(18),
	C3DiF_TexStatus        = BIT(19),

#define C3DiF_ProcTexLut(n) BIT(20+(n))
	C3DiF_ProcTexLutAll = 7 << 20,
#define C3DiF_Tex(n) BIT(23+(n))
	C3DiF_TexAll = 7 << 23,
#define C3DiF_TexEnv(n) BIT(26+(n))
	C3DiF_TexEnvAll = 0x3F << 26,
};

static inline C3D_Context* C3Di_GetContext(void)
{
	extern C3D_Context __C3D_Context;
	return &__C3D_Context;
}

static inline bool addrIsVRAM(const void* addr)
{
	u32 vaddr = (u32)addr;
	return vaddr >= OS_VRAM_VADDR && vaddr < OS_VRAM_VADDR + OS_VRAM_SIZE;
}

static inline vramAllocPos addrGetVRAMBank(const void* addr)
{
	u32 vaddr = (u32)addr;
	return vaddr < OS_VRAM_VADDR + OS_VRAM_SIZE/2 ? VRAM_ALLOC_A : VRAM_ALLOC_B;
}

void C3Di_LightMtlBlend(C3D_Light* light);

void C3Di_DirtyUniforms(GPU_SHADER_TYPE type);
void C3Di_LoadShaderUniforms(shaderInstance_s* si);
void C3Di_ClearShaderUniforms(GPU_SHADER_TYPE type);

bool C3Di_SplitFrame(u32** pBuf, u32* pOffset);

void C3Di_RenderQueueInit(gxCmdQueue_s *queue);
void C3Di_RenderQueueExit(void);
void C3Di_RenderQueueWaitDone(void);
void C3Di_RenderQueueEnableVBlank(void);
void C3Di_RenderQueueDisableVBlank(void);
