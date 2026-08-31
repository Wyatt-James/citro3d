#pragma once
#include "types.h"
#include <math.h>

enum
{
	C3DiG_BeginAcc    = BIT(0),
	C3DiG_AccStage    = BIT(1),
	C3DiG_SetAccMax   = BIT(2),
	C3DiG_RenderStage = BIT(3),
};

typedef struct
{
	u32 data[128];
} C3D_FogLut;

typedef struct
{
	u32 diff[8];
	u32 color[8];
} C3D_GasLut;

typedef struct
{
	u16 attn, accMax;
	u32 lightXY, lightZ, lightZColor;
	u32 deltaZ : 24;
	u32 flags : 8;
} C3D_GasConfig;

static inline float FogLut_CalcZ(float depth, float near, float far)
{
	return far*near/(depth*(far-near)+near);
}

void FogLut_FromArray(C3D_FogLut* lut, const float data[256]);
void FogLut_Exp(C3D_FogLut* lut, float density, float gradient, float near, float far);

void C3D_FogGasMode(GPU_FOGMODE fogMode, GPU_GASMODE gasMode, bool zFlip);
void C3D_FogColor(u32 color);
void C3D_FogLutBind(C3D_FogLut* lut);

void GasLut_FromArray(C3D_GasLut* lut, const u32 data[9]);

void C3D_GasBeginAcc(void);
void C3D_GasDeltaZ(float value);

void C3D_GasAccMax(float value);
void C3D_GasAttn(float value);
void C3D_GasLightPlanar(float min, float max, float attn);
void C3D_GasLightView(float min, float max, float attn);
void C3D_GasLightDirection(float dotp);
void C3D_GasLutInput(GPU_GASLUTINPUT input);
void C3D_GasLutBind(C3D_GasLut* lut);

static inline void C3D_SetFogGasMode(u32* texEnvBuf, GPU_FOGMODE fogMode, GPU_GASMODE gasMode, bool zFlip)
{
	*texEnvBuf &= ~0x100FF;
	*texEnvBuf |= (fogMode&7) | ((gasMode&1)<<3) | (zFlip ? BIT(16) : 0);
}
