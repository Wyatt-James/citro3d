#pragma once

#include <stdbool.h>
#include <c3d/maths.h>
#include <c3d/types.h>
#include <c3d/base.h>
#include <c3d/texture.h>
#include <c3d/fog.h>
#include <c3d/light.h>
#include <c3d/framebuffer.h>
#include <c3d/attribs.h>
#include <c3d/effect.h>
#include <c3d/texenv.h>

// Flags for controlling C3D_SendEffect
typedef enum {
	// Raw GPUCMD flags
	C3DF_EFFECT_FACECULLING_CONFIG        = BIT(0),
	C3DF_EFFECT_DEPTHMAP_SCALE_AND_OFFSET = BIT(1),
	C3DF_EFFECT_EARLYDEPTH_FUNC_AND_TEST  = BIT(2),
	C3DF_EFFECT_EARLYDEPTH_DATA           = BIT(3),
	C3DF_EFFECT_DEPTHMAP_ENABLE           = BIT(4),
	C3DF_EFFECT_COLOR_OPERATION           = BIT(5),
	C3DF_EFFECT_MISC                      = BIT(6),
	C3DF_EFFECT_EARLYDEPTH_TEST2          = BIT(7),
	C3DF_EFFECT_GAS_DELTAZ_DEPTH          = BIT(8),
	C3DF_EFFECT_FRAGOP_SHADOW             = BIT(9),
	C3DF_EFFECT_ALL                       = ((C3DF_EFFECT_FRAGOP_SHADOW << 1) - 1),
	// Flags per-function
	C3DF_EFFECT_CULLFACE                  = C3DF_EFFECT_FACECULLING_CONFIG,
	C3DF_EFFECT_DEPTHMAP                  = C3DF_EFFECT_DEPTHMAP_ENABLE | C3DF_EFFECT_DEPTHMAP_SCALE_AND_OFFSET,
	C3DF_EFFECT_STENCILTEST               = C3DF_EFFECT_MISC,
	C3DF_EFFECT_STENCILOP                 = C3DF_EFFECT_MISC,
	C3DF_EFFECT_BLENDINGCOLOR             = C3DF_EFFECT_MISC,
	C3DF_EFFECT_EARLYDEPTHTEST            = C3DF_EFFECT_EARLYDEPTH_FUNC_AND_TEST | C3DF_EFFECT_EARLYDEPTH_DATA,
	C3DF_EFFECT_DEPTHTEST                 = C3DF_EFFECT_MISC | C3DF_EFFECT_GAS_DELTAZ_DEPTH,
	C3DF_EFFECT_ALPHATEST                 = C3DF_EFFECT_MISC,
	C3DF_EFFECT_ALPHABLEND                = C3DF_EFFECT_MISC | C3DF_EFFECT_COLOR_OPERATION,
	C3DF_EFFECT_COLORLOGICOP              = C3DF_EFFECT_MISC | C3DF_EFFECT_COLOR_OPERATION,
	C3DF_EFFECT_FRAGOPMODE                = C3DF_EFFECT_COLOR_OPERATION,
	C3DF_EFFECT_FRAGOPSHADOW              = C3DF_EFFECT_FRAGOP_SHADOW,
} C3D_EffectFlags;

typedef enum {
	// Raw GPUCMD flags
	C3DF_TEXENVBUF_TEXENVBUF      = BIT(0),
	C3DF_TEXENVBUF_TEXENVBUFCOLOR = BIT(1),
	C3DF_TEXENVBUF_FOGCOLOR       = BIT(2),
	C3DF_TEXENVBUF_ALL            = ((C3DF_TEXENVBUF_FOGCOLOR << 1) - 1),
	// Flags per-function
	C3DF_TEXENVBUF_FOGGASMODE      = C3DF_TEXENVBUF_TEXENVBUF,
	C3DF_TEXENVBUF_TEXENVBUFUPDATE = C3DF_TEXENVBUF_TEXENVBUF,
 // C3DF_TEXENVBUF_FOGCOLOR        = C3DF_TEXENVBUF_FOGCOLOR,
} C3D_TexEnvBufFlags;

/**
 * Functions to send GPUCMDs
 */

void C3Di_LightEnvUpdate(C3D_LightEnv* env);

static inline void C3D_SendFrameBuf(const C3D_FrameBuf* fb, bool flushFbAndEarlyDepth)
{
    if (flushFbAndEarlyDepth)
    {
        GPUCMD_AddWrite(GPUREG_FRAMEBUFFER_FLUSH, 1);
        GPUCMD_AddWrite(GPUREG_EARLYDEPTH_CLEAR, 1);
    }
    
	static const u8 colorFmtSizes[] = {2,1,0,0,0};
	u32 param[4];
	param[0] = osConvertVirtToPhys(fb->depthBuf) >> 3;
	param[1] = osConvertVirtToPhys(fb->colorBuf) >> 3;
	param[2] = 0x01000000 | (((u32)(fb->height-1) & 0xFFF) << 12) | (fb->width & 0xFFF);

	GPUCMD_AddWrite(GPUREG_FRAMEBUFFER_INVALIDATE, 1);

	GPUCMD_AddIncrementalWrites_Auto(GPUREG_DEPTHBUFFER_LOC, param, 3);

	GPUCMD_AddWrite(GPUREG_RENDERBUF_DIM,       param[2]);
	GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT,  fb->depthFmt);
	GPUCMD_AddWrite(GPUREG_COLORBUFFER_FORMAT,  colorFmtSizes[fb->colorFmt] | ((u32)fb->colorFmt << 16));
	GPUCMD_AddWrite(GPUREG_FRAMEBUFFER_BLOCK32, fb->block32 ? 1 : 0);

	// Enable or disable color/depth buffers
	param[0] = param[1] = fb->colorBuf ? fb->colorMask : 0;
	param[2] = param[3] = fb->depthBuf ? fb->depthMask : 0;
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_COLORBUFFER_READ, param, 4);
}

static inline void C3D_SendViewport(const C3D_Viewport* viewport)
{
    GPUCMD_AddIncrementalWrites_Auto(GPUREG_VIEWPORT_WIDTH, viewport->arr, 4);
    GPUCMD_AddWrite(GPUREG_VIEWPORT_XY, viewport->position);
}

static inline void C3D_SendScissor(const C3D_Scissor* scissor)
{
    GPUCMD_AddIncrementalWrites_Auto(GPUREG_SCISSORTEST_MODE, scissor->arr, 3);
}

static inline void C3D_SendProgram(shaderProgram_s* program, bool vshCode, bool gshCode)
{
    shaderProgramConfigure(program, vshCode, gshCode);
}

static inline void C3D_SendAttrInfo(const C3D_AttrInfo* info)
{
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_ATTRIBBUFFERS_FORMAT_LOW, (u32*)info->flags, sizeof(info->flags)/sizeof(u32));
	GPUCMD_AddMaskedWrite(GPUREG_VSH_INPUTBUFFER_CONFIG, 0xB, 0xA0000000 | (info->attrCount - 1));
	GPUCMD_AddWrite(GPUREG_VSH_NUM_ATTR, info->attrCount - 1);
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_VSH_ATTRIBUTES_PERMUTATION_LOW, (u32*)&info->permutation, 2);
}

// Returns the new bufCount
static inline void C3D_SendBufInfo(const C3D_BufInfo* info, u32 bufsToWrite)
{
    GPUCMD_AddWrite(GPUREG_ATTRIBBUFFERS_LOC, info->base_paddr >> 3);
    GPUCMD_AddIncrementalWrites_Auto(GPUREG_ATTRIBBUFFER0_OFFSET, (u32*)info->buffers, (bufsToWrite * sizeof(info->buffers[0]))/sizeof(u32));
}

// For performance reasons, the flags should be passed as a constant.
static inline void C3D_SendEffect(const C3D_Effect* e, u32 flags)
{
	if (flags & C3DF_EFFECT_FACECULLING_CONFIG)
		GPUCMD_AddWrite(GPUREG_FACECULLING_CONFIG, e->cullMode & 0x3);
	
	if (flags & C3DF_EFFECT_DEPTHMAP_SCALE_AND_OFFSET)
		GPUCMD_AddIncrementalWrites_Inline(GPUREG_DEPTHMAP_SCALE, (u32*)&e->zScale, 2);

	if (flags & C3DF_EFFECT_EARLYDEPTH_FUNC_AND_TEST)
	{
		u32 data[] = {e->earlyDepthFunc, e->earlyDepth ? 1 : 0};
		GPUCMD_AddMaskedIncrementalWrites_Inline(GPUREG_EARLYDEPTH_FUNC, 1, data, sizeof(data) / sizeof(u32));
	}

	if (flags & C3DF_EFFECT_EARLYDEPTH_DATA)
		GPUCMD_AddMaskedWrite(GPUREG_EARLYDEPTH_DATA, 0x7, e->earlyDepthRef);

	if (flags & C3DF_EFFECT_DEPTHMAP_ENABLE)
		GPUCMD_AddWrite(GPUREG_DEPTHMAP_ENABLE, e->zBuffer ? 1 : 0);

	if (flags & C3DF_EFFECT_COLOR_OPERATION)
		GPUCMD_AddMaskedWrite(GPUREG_COLOR_OPERATION, 7, e->fragOpMode);

	if (flags & C3DF_EFFECT_MISC)
		GPUCMD_AddIncrementalWrites_Inline(GPUREG_BLEND_FUNC, (u32*) &e->alphaBlend, 7);

	if (flags & C3DF_EFFECT_EARLYDEPTH_TEST2)
		GPUCMD_AddWrite(GPUREG_EARLYDEPTH_TEST2, e->earlyDepth ? 1 : 0);

	if (flags & C3DF_EFFECT_GAS_DELTAZ_DEPTH)
		GPUCMD_AddMaskedWrite(GPUREG_GAS_DELTAZ_DEPTH, 0x8, (u32)GPU_MAKEGASDEPTHFUNC((e->depthTest>>4)&7) << 24);

	if (flags & C3DF_EFFECT_FRAGOP_SHADOW)
		GPUCMD_AddWrite(GPUREG_FRAGOP_SHADOW, e->fragOpShadow);
}

static inline void C3D_SendTex(u32 unit, const C3D_Tex* tex)
{
	u32 unit_border_color_reg;
	u32 unit_type_reg;

	switch (unit)
	{
		default:
			return;
		case 0:
			unit_border_color_reg = GPUREG_TEXUNIT0_BORDER_COLOR;
			unit_type_reg = GPUREG_TEXUNIT0_TYPE;
			break;
		case 1:
			unit_border_color_reg = GPUREG_TEXUNIT1_BORDER_COLOR;
			unit_type_reg = GPUREG_TEXUNIT1_TYPE;
			break;
		case 2:
			unit_border_color_reg = GPUREG_TEXUNIT2_BORDER_COLOR;
			unit_type_reg = GPUREG_TEXUNIT2_TYPE;
			break;
	}

	u32 reg[10];
	u32 regcount = 5;
	reg[0] = tex->border;
	reg[1] = tex->dim;
	reg[2] = tex->param;
	reg[3] = tex->lodParam;
	if (C3D_TexIs2D(tex))
		reg[4] = osConvertVirtToPhys(tex->data) >> 3;
	else
	{
		u32 i;
		C3D_TexCube* cube = tex->cube;
		regcount = 10;
		reg[4] = osConvertVirtToPhys(cube->data[0]) >> 3;
		for (i = 1; i < 6; i ++)
			reg[4+i] = (osConvertVirtToPhys(cube->data[i]) >> 3) & 0x3FFFFF;
	}
	
	GPUCMD_AddIncrementalWrites_Auto(unit_border_color_reg, reg, regcount);
	GPUCMD_AddWrite(unit_type_reg, tex->fmt);
}

static inline void C3D_SendTexStatus(u32 texConfig, u32 texShadow)
{
    GPUCMD_AddMaskedWrite(GPUREG_TEXUNIT_CONFIG, 0xB, texConfig);

    if (texConfig & BIT(16))
    {
        GPUCMD_AddMaskedWrite(GPUREG_TEXUNIT_CONFIG, 0x4, BIT(16)); // Clear texture cache if requested *after* configuring texture units
    }

    GPUCMD_AddWrite(GPUREG_TEXUNIT0_SHADOW, texShadow);
}

static inline void C3D_SendProcTex(void) // C3D_Context* ctx
{
    // C3Di_ProcTexUpdate(); // WYATT_TODO proctex not yet supported. Requires ctx.
}

static inline void C3D_SendTexEnvBuf(u32 texEnvBuf, u32 texEnvBufColor, u32 fogColor, u32 flags)
{
	if (flags & C3DF_TEXENVBUF_TEXENVBUF)
    	GPUCMD_AddMaskedWrite(GPUREG_TEXENV_UPDATE_BUFFER, 0x7, texEnvBuf);
	if (flags & C3DF_TEXENVBUF_TEXENVBUFCOLOR)
    	GPUCMD_AddWrite(GPUREG_TEXENV_BUFFER_COLOR, texEnvBufColor);
	if (flags & C3DF_TEXENVBUF_FOGCOLOR)
    	GPUCMD_AddWrite(GPUREG_FOG_COLOR, fogColor);
}

static inline void C3D_SendFogLut(const C3D_FogLut* fogLut)
{
    GPUCMD_AddWrite(GPUREG_FOG_LUT_INDEX, 0);
    GPUCMD_AddWrites_Auto(GPUREG_FOG_LUT_DATA0, fogLut->data, 128);
}

static inline void C3D_SendGas(const C3D_GasConfig* gasConfig)
{
    if (gasConfig->flags & C3DiG_BeginAcc)
        GPUCMD_AddMaskedWrite(GPUREG_GAS_ACCMAX_FEEDBACK, 0x3, 0);

    if (gasConfig->flags & C3DiG_AccStage)
        GPUCMD_AddMaskedWrite(GPUREG_GAS_DELTAZ_DEPTH, 0x7, gasConfig->deltaZ);

    if (gasConfig->flags & C3DiG_SetAccMax)
        GPUCMD_AddWrite(GPUREG_GAS_ACCMAX, gasConfig->accMax);

    if (gasConfig->flags & C3DiG_RenderStage)
    {
        GPUCMD_AddWrite(GPUREG_GAS_ATTENUATION, gasConfig->attn);
        GPUCMD_AddWrite(GPUREG_GAS_LIGHT_XY, gasConfig->lightXY);
        GPUCMD_AddWrite(GPUREG_GAS_LIGHT_Z, gasConfig->lightZ);
        GPUCMD_AddWrite(GPUREG_GAS_LIGHT_Z_COLOR, gasConfig->lightZColor);
    }
}

static inline void C3D_SendGasLut(const C3D_GasLut* gasLut)
{
    GPUCMD_AddWrite(GPUREG_GAS_LUT_INDEX, 0);
    GPUCMD_AddWrites_Auto(GPUREG_GAS_LUT_DATA, (u32*)gasLut, 16);
}

static inline void C3D_SendTexEnv(u32 id, const C3D_TexEnv* env)
{
    if (id >= 6) return;
	if (id >= 4) id += 2;
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_TEXENV0_SOURCE + id*8, (u32*)env, sizeof(C3D_TexEnv)/sizeof(u32));
}

static inline void C3D_SendLightEnvEnable(const C3D_LightEnv* env)
{
    u32 enable = env != NULL;
    GPUCMD_AddWrite(GPUREG_LIGHTING_ENABLE0, enable);
    GPUCMD_AddWrite(GPUREG_LIGHTING_ENABLE1, !enable);
}

static inline void C3D_SendLightEnv(C3D_LightEnv* env) // Not const!
{
    C3Di_LightEnvUpdate(env); // WYATT_TODO too ugly to untangle right now. Modifies a buncha nonsense!
}

static inline void C3D_SendFixedAttrib(const C3D_FVec* v, u32 idx)
{
    if (idx >= 12)
        return;

    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_INDEX, idx);
    C3D_ImmSendAttrib(v->x, v->y, v->z, v->w); // WYATT_TODO Could inline this
}

static inline void C3D_SendF24Unif(const float24Uniform_s* unif, GPU_SHADER_TYPE type)
{
	int offset = type == GPU_GEOMETRY_SHADER ? (GPUREG_GSH_BOOLUNIFORM-GPUREG_VSH_BOOLUNIFORM) : 0;
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_VSH_FLOATUNIFORM_CONFIG+offset, (u32*)unif, 4);
}

static inline void C3D_SendFVUnifs(GPU_SHADER_TYPE type, u32 id, const C3D_FVec* unifs, u32 num)
{
	int offset = type == GPU_GEOMETRY_SHADER ? (GPUREG_GSH_BOOLUNIFORM-GPUREG_VSH_BOOLUNIFORM) : 0;
	GPUCMD_AddWrite(GPUREG_VSH_FLOATUNIFORM_CONFIG+offset, 0x80000000 | id);
	GPUCMD_AddWrites_Auto(GPUREG_VSH_FLOATUNIFORM_DATA+offset, (u32*) unifs, num * 4);
}

static inline void C3D_SendFVUnifVec(GPU_SHADER_TYPE type, u32 id, const C3D_FVec* unif)
{
	C3D_SendFVUnifs(type, id, unif, 1);
}

static inline void C3D_SendFVUnif(GPU_SHADER_TYPE type, u32 id, float x, float y, float z, float w)
{
	C3D_FVec vec = FVec4_New(x, y, z, w);
	C3D_SendFVUnifVec(type, id, &vec);
}

static inline void C3D_SendFVUnifMtxNx4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx, int num)
{
	C3D_SendFVUnifs(type, id, (const C3D_FVec*) mtx, num);
}

static inline void C3D_SendFVUnifMtx4x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx)
{
	C3D_SendFVUnifMtxNx4(type, id, mtx, 4);
}

static inline void C3D_SendFVUnifMtx3x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx)
{
	C3D_SendFVUnifMtxNx4(type, id, mtx, 3);
}

static inline void C3D_SendFVUnifMtx2x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx)
{
	C3D_SendFVUnifMtxNx4(type, id, mtx, 2);
}

static inline void C3D_SendIntUnif(const C3D_IVec* unif, GPU_SHADER_TYPE type, u32 id)
{
	int offset = type == GPU_GEOMETRY_SHADER ? (GPUREG_GSH_BOOLUNIFORM-GPUREG_VSH_BOOLUNIFORM) : 0;
	GPUCMD_AddWrite(GPUREG_VSH_INTUNIFORM_I0+offset+id, *unif);
}

static inline void C3D_SendBoolUnifs(u16 unif, GPU_SHADER_TYPE type)
{
	int offset = type == GPU_GEOMETRY_SHADER ? (GPUREG_GSH_BOOLUNIFORM-GPUREG_VSH_BOOLUNIFORM) : 0;
	GPUCMD_AddWrite(GPUREG_VSH_BOOLUNIFORM+offset, 0x7FFF0000 | unif);
}

static inline void C3D_SendDrawArrays(GPU_Primitive_t primitive, int first, int size)
{
	// Set primitive type
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 2, primitive);
	// Start a new primitive (breaks off a triangle strip/fan)
	GPUCMD_AddWrite(GPUREG_RESTART_PRIMITIVE, 1);
	// The index buffer is not used, but this command is still required
	GPUCMD_AddWrite(GPUREG_INDEXBUFFER_CONFIG, 0x80000000);
	// Number of vertices
	GPUCMD_AddWrite(GPUREG_NUMVERTICES, size);
	// First vertex
	GPUCMD_AddWrite(GPUREG_VERTEX_OFFSET, first);
	// Enable array drawing mode
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 1, 1);
	// Enable drawing mode
	GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 0);
	// Trigger array drawing
	GPUCMD_AddWrite(GPUREG_DRAWARRAYS, 1);
	// Go back to configuration mode
	GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 1);
	// Disable array drawing mode
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 1, 0);
	// Clear the post-vertex cache
	GPUCMD_AddWrite(GPUREG_VTX_FUNC, 1);
}

// base_paddr is bufInfo.base_paddr.
static inline void C3D_SendDrawElements(GPU_Primitive_t primitive, int count, int type, const void* indices, u32 base_paddr)
{
	u32 pa = osConvertVirtToPhys(indices);
	if (pa < base_paddr) return;

	// Set primitive type
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 2, primitive != GPU_TRIANGLES ? primitive : GPU_GEOMETRY_PRIM);
	// Start a new primitive (breaks off a triangle strip/fan)
	GPUCMD_AddWrite(GPUREG_RESTART_PRIMITIVE, 1);
	// Configure the index buffer
	GPUCMD_AddWrite(GPUREG_INDEXBUFFER_CONFIG, (pa - base_paddr) | (type << 31));
	// Number of vertices
	GPUCMD_AddWrite(GPUREG_NUMVERTICES, count);
	// First vertex
	GPUCMD_AddWrite(GPUREG_VERTEX_OFFSET, 0);
	// Enable triangle element drawing mode if necessary
	if (primitive == GPU_TRIANGLES)
	{
		GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 2, 0x100);
		GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 2, 0x100);
	}
	// Enable drawing mode
	GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 0);
	// Trigger element drawing
	GPUCMD_AddWrite(GPUREG_DRAWELEMENTS, 1);
	// Go back to configuration mode
	GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 1);
	// Disable triangle element drawing mode if necessary
	if (primitive == GPU_TRIANGLES)
	{
		GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 2, 0);
		GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 2, 0);
	}
	// Clear the post-vertex cache
	GPUCMD_AddWrite(GPUREG_VTX_FUNC, 1);
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x8, 0);
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x8, 0);
}

static inline void C3D_SendImmDrawBegin(GPU_Primitive_t primitive)
{
	// Set primitive type
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 2, primitive);
	// Start a new primitive (breaks off a triangle strip/fan)
	GPUCMD_AddWrite(GPUREG_RESTART_PRIMITIVE, 1);
	// Not sure if this command is necessary
	GPUCMD_AddWrite(GPUREG_INDEXBUFFER_CONFIG, 0x80000000);
	// Enable vertex submission mode
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 1, 1);
	// Enable drawing mode
	GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 0);
	// Begin immediate-mode vertex submission
	GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_INDEX, 0xF);
}

static inline void C3D_SendImmDrawEnd(void)
{
	// Go back to configuration mode
	GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 1);
	// Disable vertex submission mode
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 1, 0);
	// Clear the post-vertex cache
	GPUCMD_AddWrite(GPUREG_VTX_FUNC, 1);
}
