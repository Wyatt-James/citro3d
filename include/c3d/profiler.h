#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
	                      --- Theory ---

	This "profiler" is intended to hook into an existing profiler.

	There are various hooks in the C3D code that, when reached, will
	call an internal function, passing a C3D_ProfilerSlot. That ID is
	then converted to a uint32_t that is passed to the External Function.
	
	                      --- State Model ---
	This profiler is intended to measure the execution times of linear
	pieces of code. It has no concept of nesting, so states cannot
	overlap. Each call to the External Function signifies the end of
	that section.

	When C3D code begins executing, there is no guarantee of any
	particular 

	WYATT_TODO finish this description. Maybe introduce a system for
	nesting?
*/

// Log slots available for profiling.
typedef enum
{
	C3D_ProfilerSlot_Misc,                // A miscellaneous state. Should be set when acquiring control from external code.
	C3D_ProfilerSlot_FrameBuf,            // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_Viewport,            // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_Scissor,             // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_Program,             // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_AttrInfo,            // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_BufInfo,             // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_Effect,              // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_TexAll,              // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_TexStatus,           // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_ProcTex,             // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_TexEnvBuf,           // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_FogLut,              // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_Gas,                 // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_TexEnvAll,           // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_LightEnv,            // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_FixedAttribDirty,    // Part of C3Di_UpdateContext
	C3D_ProfilerSlot_UpdateUniforms,      // Called at the end of C3Di_UpdateContext and callable externally
	C3D_ProfilerSlot_ImmediateDraw,       // The time taken for C3D_ImmediateDraw, excluding updating context
	C3D_ProfilerSlot_DrawArrays,          // The time taken for C3D_DrawArrays, excluding updating context
	C3D_ProfilerSlot_DrawElements,        // The time taken for C3D_DrawElements, excluding updating context
	C3D_ProfilerSlot_Count                // Number of log slots
} C3D_ProfilerSlot;

/*
	Sets the profiler function to use, or NULL to disable.
	This function must accept any valid u32 as its slot ID.
*/ 
void C3D_ProfilerFunc(void (*profiler_func)(uint32_t));

// Sets the internal->external category mapping for a C3D_ProfilerSlot.
void C3D_ProfilerCategoryMap(C3D_ProfilerSlot c3d_category, uint32_t external_category);

// Enables or disables a profiling category.
void C3D_ProfilerCategoryEnable(C3D_ProfilerSlot c3d_category, bool enabled);

// Initializes all profiler categories sequentially, starting from the given external ID.
void C3D_ProfilerCategoryClearAll(uint32_t external_id);

// Initializes all profiler categories sequentially, starting from the given external ID.
void C3D_ProfilerCategoryMapAll(uint32_t starting_external_id);

// Enables or disables all profiler categories.
void C3D_ProfilerCategoryEnableAll(bool enable);

// Initializes the given profiler categories, sequentially, starting from the given external ID.
void C3D_ProfilerCategoryMapMultiple(C3D_ProfilerSlot categories[], size_t num_categories, uint32_t starting_external_id);

// Enables or disables the given profiler categories.
void C3D_ProfilerCategoryEnableMultiple(C3D_ProfilerSlot categories[], size_t num_categories, bool enable);
