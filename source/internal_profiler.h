#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "c3d/profiler.h"

/*
	Wraps a statement in the given profiler block.
	Note that a profiler block does not create its own scope.

	Example:
	C3D_PROFILE_BLOCK(C3D_ProfilerSlot_FrameBuf,
		func_to_profile();
		another_func();
	);
*/
#define C3D_PROFILE_BLOCK(category_, stmt_) \
	C3Di_Profile_Enter_Block(category_);    \
	stmt_                                   \
	C3Di_Profile_Exit_Block()               \

typedef struct
{
	uint8_t external_id;
	bool enabled;
} C3D_ProfilerSlotSettings;

typedef struct
{
	void (*profiler_func)(uint32_t external_id);                    // Can be null. Must accept any valid u32.
	C3D_ProfilerSlotSettings log_settings[C3D_ProfilerSlot_Count];  // Per-slot settings
	C3D_ProfilerSlot current_block_category;                        // The current C3D_ProfilerSlot. Used to optimize sequental blocks.
	bool log_slot_skipped;								            // Used to optimize sequential blocks.
} C3D_Profiler;

#ifdef CITRO3D_BUILD_PROFILER // If the flag is enabled, don't stub the profiler functions.

static inline C3D_Profiler* C3Di_GetProfiler(void)
{
	extern C3D_Profiler __C3D_Profiler;
	return &__C3D_Profiler;
}

// Immediately profile a category. Should be called when acquiring control from unknown code.
static inline void C3Di_Profile(C3D_ProfilerSlot category)
{
	C3D_Profiler* p = C3Di_GetProfiler();
    if (category < C3D_ProfilerSlot_Count        &&
		p->profiler_func != NULL            &&
		p->log_settings[category].enabled)
	{
        p->profiler_func(p->log_settings[category].external_id);
		return true;
	}
	return false;
}

// Call to begin a profiler block
static inline void C3Di_Profile_Enter_Block(C3D_ProfilerSlot category)
{

	C3D_Profiler* p = C3Di_GetProfiler();
	if (p->log_settings[category].enabled) {
		if (p->log_slot_skipped)
			C3Di_Profile(C3D_ProfilerSlot_Misc); // Only reset if we skipped a slot
		p->log_slot_skipped = false;
		p->current_block_category = category;
	} else
		p->log_slot_skipped = true;
}

// Call to end a profiler block
static inline void C3Di_Profile_Exit_Block()
{
	C3Di_Profile(C3Di_GetProfiler()->current_block_category);
}

#else // CITRO3D_BUILD_PROFILER // If the flag is disabled, stub the profiler functions.

static inline C3D_Profiler* C3Di_GetProfiler(void)
{
    return &(C3D_Profiler) {
        .profiler_func          = NULL,
        .log_settings           = {{}},
        .current_block_category = C3D_ProfilerSlot_Count,
        .log_slot_skipped       = false,
    };
}

static inline void C3Di_Profile(C3D_ProfilerSlot category)              {}
static inline void C3Di_Profile_Enter_Block(C3D_ProfilerSlot category)  {}
static inline void C3Di_Profile_Exit_Block()                       {}
#endif
