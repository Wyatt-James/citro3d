#include "internal.h"
#include <stdarg.h>

#define NUMARGS(...)  (sizeof((gpucmd_single_t[]){__VA_ARGS__})/sizeof(gpucmd_single_t))
#define GPUCMD_AddBatch_V(...)  (GPUCMD_AddBatchOfSingles_Int(NUMARGS(__VA_ARGS__), (gpucmd_single_t[]) {__VA_ARGS__}))

void C3D_DrawArrays(GPU_Primitive_t primitive, int first, int size)
{
	C3Di_UpdateContext();

	C3Di_Profile_Enter_Block(C3D_ProfilerSlot_DrawArrays);

	// GPUCMD_AddBatch_V(
	// 	// Set primitive type
	// 	GPUCMD_MaskedSingle(GPUREG_PRIMITIVE_CONFIG, 2, primitive),
	// 	// Start a new primitive (breaks off a triangle strip/fan)
	// 	GPUCMD_Single(GPUREG_RESTART_PRIMITIVE, 1),
	// 	// The index buffer is not used, but this command is still required
	// 	GPUCMD_Single(GPUREG_INDEXBUFFER_CONFIG, 0x80000000),
	// 	// Number of vertices
	// 	GPUCMD_Single(GPUREG_NUMVERTICES, size),
	// 	// First vertex
	// 	GPUCMD_Single(GPUREG_VERTEX_OFFSET, first),
	// 	// Enable array drawing mode
	// 	GPUCMD_MaskedSingle(GPUREG_GEOSTAGE_CONFIG2, 1, 1),
	// 	// Enable drawing mode
	// 	GPUCMD_MaskedSingle(GPUREG_START_DRAW_FUNC0, 1, 0),
	// 	// Trigger array drawing
	// 	GPUCMD_Single(GPUREG_DRAWARRAYS, 1),
	// 	// Go back to configuration mode
	// 	GPUCMD_MaskedSingle(GPUREG_START_DRAW_FUNC0, 1, 1),
	// 	// Disable array drawing mode
	// 	GPUCMD_MaskedSingle(GPUREG_GEOSTAGE_CONFIG2, 1, 0),
	// 	// Clear the post-vertex cache
	// 	GPUCMD_Single(GPUREG_VTX_FUNC, 1)
	// );

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

	C3Di_GetContext()->flags |= C3DiF_DrawUsed;

	C3Di_Profile_Exit_Block();
}
