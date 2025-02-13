#pragma once
#include "maths.h"
#include <stdint.h>

#define C3D_FVUNIF_COUNT 96
#define C3D_IVUNIF_COUNT 4

#define C3D_SET_LSB(num_bits_) ((1UL << (num_bits_)) - 1)    // Sets N least significant bits.
#define C3D_SET_MSB(t_, num_bits_) (~((t_)0) << (num_bits_)) // Sets N most significant bits of a u32.
#define C3D_FVUNIF_DIRTY_ARRAY_LENGTH 3

extern C3D_FVec C3D_FVUnif[2][C3D_FVUNIF_COUNT];
extern C3D_IVec C3D_IVUnif[2][C3D_IVUNIF_COUNT];
extern u16      C3D_BoolUnifs[2];

extern u32  C3D_FVUnifDirty[2][C3D_FVUNIF_DIRTY_ARRAY_LENGTH]; // 96-bit bitfield
extern bool C3D_IVUnifDirty[2][C3D_IVUNIF_COUNT];
extern bool C3D_BoolUnifsDirty[2];

// Marks the given registers in the bitfield as dirty
static inline void C3D_RegDirty(u32 bitfield[C3D_FVUNIF_DIRTY_ARRAY_LENGTH], int id, int size)
{
	const u32 lowest_word = id / 32;
	const u32 highest_word = (id + size - 1) / 32;

	if (lowest_word == highest_word)
		bitfield[lowest_word] |= (C3D_SET_LSB(size) << id);
	else
		for (u32 w = lowest_word; w <= highest_word; w++) {
			if (w == lowest_word)
				bitfield[w] |= C3D_SET_MSB(u32, id - w * 32);   // Set upper bits of lowest word
			else if (w == highest_word)
				bitfield[w] |= C3D_SET_LSB(id + size - w * 32); // Set lower bits of highest word
			else
				bitfield[w] = ~0;                           // Fill intermediate words completely
		}
}

// Marks the given registers in the bitfield as clean
static inline void C3D_RegClean(u32 bitfield[C3D_FVUNIF_DIRTY_ARRAY_LENGTH], int id, int size)
{
	const u32 lowest_word = id / 32;
	const u32 highest_word = (id + size - 1) / 32;

	if (lowest_word == highest_word)
		bitfield[lowest_word] &= ~(C3D_SET_LSB(size) << id);
	else
		for (u32 w = lowest_word; w <= highest_word; w++) {
			if (w == lowest_word)
				bitfield[w] &= ~C3D_SET_MSB(u32, id - w * 32);   // Clear upper bits of lowest word
			else if (w == highest_word)
				bitfield[w] &= ~C3D_SET_LSB(id + size - w * 32); // Clear lower bits of highest word
			else
				bitfield[w] = 0;                             // Zero intermediate words completely
		}
}

static inline C3D_FVec* C3D_FVUnifWritePtr(GPU_SHADER_TYPE type, int id, int size)
{
	C3D_RegDirty(C3D_FVUnifDirty[type], id, size);
	return &C3D_FVUnif[type][id];
}

static inline C3D_IVec* C3D_IVUnifWritePtr(GPU_SHADER_TYPE type, int id)
{
	id -= 0x60;
	C3D_IVUnifDirty[type][id] = true;
	return &C3D_IVUnif[type][id];
}

static inline void C3D_FVUnifMtxNx4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx, int num)
{
	int i;
	C3D_FVec* ptr = C3D_FVUnifWritePtr(type, id, num);
	for (i = 0; i < num; i ++)
		ptr[i] = mtx->r[i]; // Struct copy.
}

static inline void C3D_FVUnifMtx4x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx)
{
	C3D_FVUnifMtxNx4(type, id, mtx, 4);
}

static inline void C3D_FVUnifMtx3x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx)
{
	C3D_FVUnifMtxNx4(type, id, mtx, 3);
}

static inline void C3D_FVUnifMtx2x4(GPU_SHADER_TYPE type, int id, const C3D_Mtx* mtx)
{
	C3D_FVUnifMtxNx4(type, id, mtx, 2);
}

static inline void C3D_FVUnifSet(GPU_SHADER_TYPE type, int id, float x, float y, float z, float w)
{
	C3D_FVec* ptr = C3D_FVUnifWritePtr(type, id, 1);
	ptr->x = x;
	ptr->y = y;
	ptr->z = z;
	ptr->w = w;
}

static inline void C3D_IVUnifSet(GPU_SHADER_TYPE type, int id, int x, int y, int z, int w)
{
	C3D_IVec* ptr = C3D_IVUnifWritePtr(type, id);
	*ptr = IVec_Pack(x, y, z, w);
}

static inline void C3D_BoolUnifSet(GPU_SHADER_TYPE type, int id, bool value)
{
	id -= 0x68;
	C3D_BoolUnifsDirty[type] = true;
	if (value)
		C3D_BoolUnifs[type] |= BIT(id);
	else
		C3D_BoolUnifs[type] &= ~BIT(id);
}

void C3D_UpdateUniforms(GPU_SHADER_TYPE type);
