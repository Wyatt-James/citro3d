#include "internal.h"
#include <stddef.h>
#include <c3d/profiling.h>

C3D_Profiler __C3D_Profiler = {
    .profiling_func = NULL,
    .log_settings = {{}},
    .current_block_category = C3D_LogSlot_Count,
    .log_slot_skipped = false,
};

void C3D_ProfilerFunc(void (*new_profiling_func)(uint8_t))
{
    __C3D_Profiler.profiling_func = new_profiling_func;
}

void C3D_ProfilerCategoryMapping(C3D_LogSlot log_slot, uint8_t external_id)
{
    __C3D_Profiler.log_settings[log_slot].external_id = external_id;
}

void C3D_ProfilingCategoryEnable(C3D_LogSlot slot, bool enabled)
{
    __C3D_Profiler.log_settings[slot].enabled = enabled;
}

void C3D_ProfilerCategoriesInit(uint8_t from)
{
    for (C3D_LogSlot i = 0; i < C3D_LogSlot_Count; i++)
        C3D_ProfilerCategoryMapping(i, from++);
}
