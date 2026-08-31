#include "internal.h"
#include <c3d/gpucmd.h>
#include <stdarg.h>

void C3D_DrawArrays(GPU_Primitive_t primitive, int first, int size)
{
	C3D_UpdateContext();

	C3Di_Profile_Enter_Block(C3D_ProfilerSlot_DrawArrays);

	C3D_SendDrawArrays(primitive, first, size);
	C3Di_GetContext()->flags |= C3DiF_DrawUsed;
	
	C3Di_Profile_Exit_Block();
}
