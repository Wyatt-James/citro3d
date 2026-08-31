#include "internal.h"
#include <c3d/gpucmd.h>

void C3D_ImmDrawBegin(GPU_Primitive_t primitive)
{
	C3D_UpdateContext();
	C3D_SendImmDrawBegin(primitive);
}

static inline void write24(u8* p, u32 val)
{
	p[0] = val;
	p[1] = val>>8;
	p[2] = val>>16;
}

void C3D_ImmSendAttrib(float x, float y, float z, float w)
{
	union
	{
		u32 packed[3];
		struct
		{
			u8 x[3];
			u8 y[3];
			u8 z[3];
			u8 w[3];
		};
	} param;

	// Convert the values to float24
	write24(param.y, f32tof24(y));
	write24(param.x, f32tof24(x));
	write24(param.z, f32tof24(z));
	write24(param.w, f32tof24(w));

	// Reverse the packed words
	u32 p = param.packed[0];
	param.packed[0] = param.packed[2];
	param.packed[2] = p;

	// Send the attribute
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_FIXEDATTRIB_DATA0, param.packed, 3);
}

void C3D_ImmDrawEnd(void)
{
	C3D_SendImmDrawEnd();
	C3Di_GetContext()->flags |= C3DiF_DrawUsed;
}
