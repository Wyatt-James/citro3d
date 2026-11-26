#pragma once
#include "buffers.h"
#include "maths.h"

#define C3D_DEFAULT_CMDBUF_SIZE 0x40000

typedef union
{
	u32 arr[5];
	struct {
		u32 width, inverted_width, height, inverted_height, position;
	};
} C3D_Viewport;

typedef union
{
	u32 arr[3];
	struct {
		u32 mode, position, dimensions;
	};
} C3D_Scissor;

typedef struct
{
	const u32 * buf;
	u32 size, offset;
} C3D_GpuCmdBuffer;

enum
{
	C3D_UNSIGNED_BYTE = 0,
	C3D_UNSIGNED_SHORT = 1,
};

bool C3D_InitEx(size_t cmdBufSize, size_t gxQueueSize, bool doubleBuf);
bool C3D_Init(size_t cmdBufSize);
void C3D_Fini(void);

float C3D_GetCmdBufUsage(void);

void C3D_BindProgram(shaderProgram_s* program);

void C3D_SetViewport(u32 x, u32 y, u32 w, u32 h);
void C3D_SetScissor(GPU_SCISSORMODE mode, u32 left, u32 top, u32 right, u32 bottom);

void C3D_DrawArrays(GPU_Primitive_t primitive, int first, int size);
void C3D_DrawElements(GPU_Primitive_t primitive, int count, int type, const void* indices);

// Immediate-mode vertex submission
void C3D_ImmDrawBegin(GPU_Primitive_t primitive);
void C3D_ImmSendAttrib(float x, float y, float z, float w);
void C3D_ImmDrawEnd(void);
void C3D_NotifyDrawUsed(void); // Must be called after drawing
void C3D_ForceContextDirty(void); // Forces all context to be fully re-sent next C3D_UpdateContext.
void C3D_UpdateContext(void); // Sends context state to GPU, subject to its dirty status.
void C3D_SetRestoreHook(void (*hook) (void)); // Called upon restoring from an APT suspend. Remove by sending NULL.
C3D_GpuCmdBuffer C3D_GetLastGpuCmdBuffer(void); // For debugging. Returns a copy.

static inline void C3D_ImmDrawRestartPrim(void)
{
	GPUCMD_AddWrite(GPUREG_RESTART_PRIMITIVE, 1);
}

// Fixed vertex attributes
C3D_FVec* C3D_FixedAttribGetWritePtr(int id);

static inline void C3D_FixedAttribSet(int id, float x, float y, float z, float w)
{
	C3D_FVec* ptr = C3D_FixedAttribGetWritePtr(id);
	ptr->x = x;
	ptr->y = y;
	ptr->z = z;
	ptr->w = w;
}

static inline void C3D_SetViewportVal(C3D_Viewport* viewport, u32 x, u32 y, u32 w, u32 h)
{
	viewport->width           = f32tof24(w / 2.0f);
	viewport->inverted_width  = f32tof31(2.0f / w) << 1;
	viewport->height          = f32tof24(h / 2.0f);
	viewport->inverted_height = f32tof31(2.0f / h) << 1;
	viewport->position        = (y << 16) | (x & 0xFFFF);
}

static inline void C3D_SetScissorVal(C3D_Scissor* scissor, GPU_SCISSORMODE mode, u32 left, u32 top, u32 right, u32 bottom)
{
	scissor->mode = mode;
	// if (mode == GPU_SCISSOR_DISABLE) return;
	scissor->position = (top << 16) | (left & 0xFFFF);
	scissor->dimensions = ((bottom-1) << 16) | ((right-1) & 0xFFFF);
}

static inline u32 C3D_GetScissorLeft(const C3D_Scissor* scissor)   {return scissor->position & UINT16_MAX;}
static inline u32 C3D_GetScissorTop(const C3D_Scissor* scissor)    {return scissor->position >> 16;}
static inline u32 C3D_GetScissorRight(const C3D_Scissor* scissor)  {return (scissor->dimensions & UINT16_MAX) + 1;}
static inline u32 C3D_GetScissorBottom(const C3D_Scissor* scissor) {return (scissor->dimensions >> 16) + 1;}
