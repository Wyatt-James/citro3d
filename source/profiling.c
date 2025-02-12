#include "internal.h"
#include <stddef.h>
#include <c3d/profiler.h>

C3D_Profiler __C3D_Profiler = {
    .profiler_func = NULL,
    .log_settings = {{}},
    .current_block_category = C3D_LogSlot_Count,
    .log_slot_skipped = false,
};

void C3D_ProfilerFunc(void (*new_profiler_func)(uint32_t))
{
    C3Di_GetProfiler()->profiler_func = new_profiler_func;
}

void C3D_ProfilerCategoryMap(C3D_LogSlot log_slot, uint32_t external_id)
{
    C3Di_GetProfiler()->log_settings[log_slot].external_id = external_id;
}

void C3D_ProfilerCategoryEnable(C3D_LogSlot slot, bool enabled)
{
    C3Di_GetProfiler()->log_settings[slot].enabled = enabled;
}

void C3D_ProfilerCategoryClearAll(uint32_t clear_id)
{
    for (C3D_LogSlot i = 0; i < C3D_LogSlot_Count; i++)
        C3D_ProfilerCategoryMap(i, clear_id);
}

void C3D_ProfilerCategoryMapAll(uint32_t starting_id)
{
    for (C3D_LogSlot i = 0; i < C3D_LogSlot_Count; i++)
        C3D_ProfilerCategoryMap(i, starting_id++);
}

void C3D_ProfilerCategoryEnableAll(bool enable)
{
    for (C3D_LogSlot i = 0; i < C3D_LogSlot_Count; i++)
        C3D_ProfilerCategoryEnable(i, enable);
}

void C3D_ProfilerCategoryMapMultiple(C3D_LogSlot categories[], size_t num_categories, uint32_t starting_id)
{
    for (int i = 0; i < num_categories; i++)
        C3D_ProfilerCategoryMap(categories[i], starting_id++);
}

void C3D_ProfilerCategoryEnableMultiple(C3D_LogSlot categories[], size_t num_categories, bool enable)
{
    for (int i = 0; i < num_categories; i++)
        C3D_ProfilerCategoryEnable(categories[i], enable);
}

