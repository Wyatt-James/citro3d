#include "internal.h"
#include <c3d/gpucmd.h>

void C3D_DrawElements(GPU_Primitive_t primitive, int count, int type, const void* indices)
{
	C3D_Context* ctx = C3Di_GetContext();
	u32 base = ctx->bufInfo.base_paddr;

	C3D_UpdateContext();

	C3Di_Profile_Enter_Block(C3D_ProfilerSlot_DrawElements);
	
	C3D_SendDrawElements(primitive, count, type, indices, base);
	ctx->flags |= C3DiF_DrawUsed;
	
	C3Di_Profile_Exit_Block();
}
