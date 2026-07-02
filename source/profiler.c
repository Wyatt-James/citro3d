#include <stddef.h>
#include <stdint.h>

#include "internal.h"
#include "c3d/profiler.h"

#ifdef CITRO3D_BUILD_PROFILER

C3D_Profiler __C3D_Profiler = {
    .profiler_func = NULL,
    .log_settings = {{}},
    .current_block_category = C3D_ProfilerSlot_Count,
    .log_slot_skipped = false,
};

void C3D_ProfilerFunc(void (*profiler_func)(uint32_t))
{
    C3Di_GetProfiler()->profiler_func = profiler_func;
}

void C3D_ProfilerCategoryMap(C3D_ProfilerSlot c3d_category, uint32_t external_category)
{
    if (c3d_category >= C3D_ProfilerSlot_Misc && c3d_category < C3D_ProfilerSlot_Count)
        C3Di_GetProfiler()->log_settings[c3d_category].external_id = external_category;
}

void C3D_ProfilerCategoryEnable(C3D_ProfilerSlot c3d_category, bool enabled)
{
    if (c3d_category >= C3D_ProfilerSlot_Misc && c3d_category < C3D_ProfilerSlot_Count)
        C3Di_GetProfiler()->log_settings[c3d_category].enabled = enabled;
}

void C3D_ProfilerCategoryClearAll(uint32_t external_id)
{
    for (C3D_ProfilerSlot i = 0; i < C3D_ProfilerSlot_Count; i++)
        C3D_ProfilerCategoryMap(i, external_id);
}

void C3D_ProfilerCategoryMapAll(uint32_t starting_external_id)
{
    for (C3D_ProfilerSlot i = 0; i < C3D_ProfilerSlot_Count; i++)
        C3D_ProfilerCategoryMap(i, starting_external_id++);
}

void C3D_ProfilerCategoryEnableAll(bool enable)
{
    for (C3D_ProfilerSlot i = 0; i < C3D_ProfilerSlot_Count; i++)
        C3D_ProfilerCategoryEnable(i, enable);
}

void C3D_ProfilerCategoryMapMultiple(C3D_ProfilerSlot categories[], size_t num_categories, uint32_t starting_external_id)
{
    for (int i = 0; i < num_categories; i++)
        C3D_ProfilerCategoryMap(categories[i], starting_external_id++);
}

void C3D_ProfilerCategoryEnableMultiple(C3D_ProfilerSlot categories[], size_t num_categories, bool enable)
{
    for (int i = 0; i < num_categories; i++)
        C3D_ProfilerCategoryEnable(categories[i], enable);
}

#else // CITRO3D_BUILD_PROFILER

// Stub all funcs and don't create a profiler struct

void C3D_ProfilerFunc(void (*profiler_func)(uint32_t))                                                                   {}
void C3D_ProfilerCategoryMap(C3D_ProfilerSlot c3d_category, uint32_t external_category)                                    {}
void C3D_ProfilerCategoryEnable(C3D_ProfilerSlot c3d_category, bool enabled)                                               {}
void C3D_ProfilerCategoryClearAll(uint32_t external_id)                                                                  {}
void C3D_ProfilerCategoryMapAll(uint32_t starting_external_id)                                                           {}
void C3D_ProfilerCategoryEnableAll(bool enable)                                                                          {}
void C3D_ProfilerCategoryMapMultiple(C3D_ProfilerSlot categories[], size_t num_categories, uint32_t starting_external_id)  {}
void C3D_ProfilerCategoryEnableMultiple(C3D_ProfilerSlot categories[], size_t num_categories, bool enable)                 {}
#endif
