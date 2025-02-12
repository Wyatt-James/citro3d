#pragma once

#include "types.h"
#include "stdbool.h"

typedef enum
{
	C3D_LogSlot_Default,             // A miscellaneous state. Should be used when acquiring control from external code.
	C3D_LogSlot_FrameBuf,            // C3DiF
	C3D_LogSlot_Viewport,            // C3DiF
	C3D_LogSlot_Scissor,             // C3DiF
	C3D_LogSlot_Program,             // C3DiF
	C3D_LogSlot_AttrInfo,            // C3DiF
	C3D_LogSlot_BufInfo,             // C3DiF
	C3D_LogSlot_Effect,              // C3DiF
	C3D_LogSlot_TexAll,              // C3DiF
	C3D_LogSlot_TexStatus,           // C3DiF
	C3D_LogSlot_ProcTex,             // C3DiF
	C3D_LogSlot_TexEnvBuf,           // C3DiF
	C3D_LogSlot_FogLut,              // C3DiF
	C3D_LogSlot_Gas,                 // Whenever gas is enabled
	C3D_LogSlot_TexEnvAll,           // C3DiF
	C3D_LogSlot_LightEnv,            // C3DiF
	C3D_LogSlot_FixedAttribDirty,    // 
	C3D_LogSlot_UpdateUniforms,      // End of C3Di_UpdateContext
	C3D_LogSlot_ImmediateDraw,      // End of C3Di_UpdateContext
	C3D_LogSlot_DrawArrays,      // End of C3Di_UpdateContext
	C3D_LogSlot_DrawElements,      // End of C3Di_UpdateContext
	C3D_LogSlot_Count                // Number of log slots
} C3D_LogSlot;

/*
	Sets the profiler function to use, or NULL to disable.
	This function must accept any valid u32 as its slot ID.
*/ 
void C3D_ProfilerFunc(void (*profiling_func)(uint32_t));

// Sets the category mapping for a C3D_LogSlot.
void C3D_ProfilerCategoryMapping(C3D_LogSlot log_slot, uint32_t profiling_category);

// Enables or disables a profiling category.
void C3D_ProfilingCategoryEnable(C3D_LogSlot slot, bool enabled);

// A convenience function that initializes all profiler categories sequentially (inclusive)
void C3D_ProfilerCategoriesInit(uint32_t from);
