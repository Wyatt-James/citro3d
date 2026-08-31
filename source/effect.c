#include "internal.h"

C3D_Effect* C3D_GetEffect(void)
{
	C3D_Context* ctx = C3Di_GetContext();
	ctx->flags |= C3DiF_Effect;
	return &ctx->effect;
}

void C3D_EffectInit(C3D_Effect* effect)
{
	// TODO: replace with direct struct access
	C3D_SetDepthMap(effect, true, -1.0f, 0.0f);
	C3D_SetCullFace(effect, GPU_CULL_BACK_CCW);
	C3D_SetStencilTest(effect, false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
	C3D_SetStencilOp(effect, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
	C3D_SetBlendingColor(effect, 0);
	C3D_SetEarlyDepthTest(effect, false, GPU_EARLYDEPTH_GREATER, 0);
	C3D_SetDepthTest(effect, true, GPU_GREATER, GPU_WRITE_ALL);
	C3D_SetAlphaTest(effect, false, GPU_ALWAYS, 0x00);
	C3D_SetAlphaBlend(effect, GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	C3D_SetFragOpMode(effect, GPU_FRAGOPMODE_GL);
	C3D_SetFragOpShadow(effect, 0.0, 1.0);
}

void C3D_DepthMap(bool bIsZBuffer, float zScale, float zOffset)
{
	C3D_SetDepthMap(C3D_GetEffect(), bIsZBuffer, zScale, zOffset);
}

void C3D_CullFace(GPU_CULLMODE mode)
{
	C3D_SetCullFace(C3D_GetEffect(), mode);
}

void C3D_StencilTest(bool enable, GPU_TESTFUNC function, int ref, int inputMask, int writeMask)
{
	C3D_SetStencilTest(C3D_GetEffect(), enable, function, ref, inputMask, writeMask);
}

void C3D_StencilOp(GPU_STENCILOP sfail, GPU_STENCILOP dfail, GPU_STENCILOP pass)
{
	C3D_SetStencilOp(C3D_GetEffect(), sfail, dfail, pass);
}

void C3D_BlendingColor(u32 color)
{
	C3D_SetBlendingColor(C3D_GetEffect(), color);
}

void C3D_EarlyDepthTest(bool enable, GPU_EARLYDEPTHFUNC function, u32 ref)
{
	C3D_SetEarlyDepthTest(C3D_GetEffect(), enable, function, ref);
}

void C3D_DepthTest(bool enable, GPU_TESTFUNC function, GPU_WRITEMASK writemask)
{
	C3D_SetDepthTest(C3D_GetEffect(), enable, function, writemask);
}

void C3D_AlphaTest(bool enable, GPU_TESTFUNC function, int ref)
{
	C3D_SetAlphaTest(C3D_GetEffect(), enable, function, ref);
}

void C3D_AlphaBlend(GPU_BLENDEQUATION colorEq, GPU_BLENDEQUATION alphaEq, GPU_BLENDFACTOR srcClr, GPU_BLENDFACTOR dstClr, GPU_BLENDFACTOR srcAlpha, GPU_BLENDFACTOR dstAlpha)
{
	C3D_SetAlphaBlend(C3D_GetEffect(), colorEq, alphaEq, srcClr, dstClr, srcAlpha, dstAlpha);
}

void C3D_ColorLogicOp(GPU_LOGICOP op)
{
	C3D_SetColorLogicOp(C3D_GetEffect(), op);
}

void C3D_FragOpMode(GPU_FRAGOPMODE mode)
{
	C3D_SetFragOpMode(C3D_GetEffect(), mode);
}

void C3D_FragOpShadow(float scale, float bias)
{
	C3D_SetFragOpShadow(C3D_GetEffect(), scale, bias);
}

