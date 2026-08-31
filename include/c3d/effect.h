#pragma once
#include "types.h"

typedef struct
{
	u32 fragOpMode;
	u32 fragOpShadow;
	u32 zScale, zOffset;
	GPU_CULLMODE cullMode;
	bool zBuffer, earlyDepth;
	GPU_EARLYDEPTHFUNC earlyDepthFunc;
	u32 earlyDepthRef;

	// Keep the order of these
	u32 alphaBlend;
	GPU_LOGICOP clrLogicOp;
	u32 blendClr;
	u32 alphaTest;
	u32 stencilMode;
	u32 stencilOp;
	u32 depthTest;

} C3D_Effect;

C3D_Effect* C3D_GetEffect(void);
void C3D_EffectInit(C3D_Effect* effect);

void C3D_DepthMap(bool bIsZBuffer, float zScale, float zOffset);
void C3D_CullFace(GPU_CULLMODE mode);
void C3D_StencilTest(bool enable, GPU_TESTFUNC function, int ref, int inputMask, int writeMask);
void C3D_StencilOp(GPU_STENCILOP sfail, GPU_STENCILOP dfail, GPU_STENCILOP pass);
void C3D_BlendingColor(u32 color);
void C3D_EarlyDepthTest(bool enable, GPU_EARLYDEPTHFUNC function, u32 ref);
void C3D_DepthTest(bool enable, GPU_TESTFUNC function, GPU_WRITEMASK writemask);
void C3D_AlphaTest(bool enable, GPU_TESTFUNC function, int ref);
void C3D_AlphaBlend(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq, GPU_BLENDFACTOR srcClr, GPU_BLENDFACTOR dstClr, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha);
void C3D_ColorLogicOp(GPU_LOGICOP op);
void C3D_FragOpMode(GPU_FRAGOPMODE mode);
void C3D_FragOpShadow(float scale, float bias);

static inline void C3D_SetDepthMap(C3D_Effect* e, bool bIsZBuffer, float zScale, float zOffset)
{
	e->zBuffer = bIsZBuffer;
	e->zScale  = f32tof24(zScale);
	e->zOffset = f32tof24(zOffset);
}

static inline void C3D_SetCullFace(C3D_Effect* e, GPU_CULLMODE mode)
{
	e->cullMode = mode;
}

static inline void C3D_SetStencilTest(C3D_Effect* e, bool enable, GPU_TESTFUNC function, int ref, int inputMask, int writeMask)
{
	e->stencilMode = (!!enable) | ((function & 7) << 4) | (writeMask << 8) | (ref << 16) | (inputMask << 24);
}

static inline void C3D_SetStencilOp(C3D_Effect* e, GPU_STENCILOP sfail, GPU_STENCILOP dfail, GPU_STENCILOP pass)
{
	e->stencilOp = sfail | (dfail << 4) | (pass << 8);
}

static inline void C3D_SetBlendingColor(C3D_Effect* e, u32 color)
{
	e->blendClr = color;
}

static inline void C3D_SetEarlyDepthTest(C3D_Effect* e, bool enable, GPU_EARLYDEPTHFUNC function, u32 ref)
{
	e->earlyDepth = enable;
	e->earlyDepthFunc = function;
	e->earlyDepthRef = ref;
}

static inline void C3D_SetDepthTest(C3D_Effect* e, bool enable, GPU_TESTFUNC function, GPU_WRITEMASK writemask)
{
	e->depthTest = (!!enable) | ((function & 7) << 4) | (writemask << 8);
}

static inline void C3D_SetAlphaTest(C3D_Effect* e, bool enable, GPU_TESTFUNC function, int ref)
{
	e->alphaTest = (!!enable) | ((function & 7) << 4) | (ref << 8);
}

static inline void C3D_SetAlphaBlend(C3D_Effect* e, GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq, GPU_BLENDFACTOR srcClr, GPU_BLENDFACTOR dstClr, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha)
{
	e->alphaBlend = colorEq | (alphaEq << 8) | (srcClr << 16) | (dstClr << 20) | (srcAlpha << 24) | (dstAlpha << 28);
	e->fragOpMode &= ~0xFF00;
	e->fragOpMode |= 0x0100;
}

static inline void C3D_SetColorLogicOp(C3D_Effect* e, GPU_LOGICOP op)
{
	e->fragOpMode &= ~0xFF00;
	e->clrLogicOp = op;
}

static inline void C3D_SetFragOpMode(C3D_Effect* e, GPU_FRAGOPMODE mode)
{
	e->fragOpMode &= ~0xFF00FF;
	e->fragOpMode |= 0xE40000 | mode;
}

static inline void C3D_SetFragOpShadow(C3D_Effect* e, float scale, float bias)
{
	e->fragOpShadow = f32tof16(scale+bias) | (f32tof16(-scale)<<16);
}
