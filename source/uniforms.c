#include "internal.h"
#include <c3d/uniforms.h>
#include <stdint.h>
#include <stddef.h>

#define NUM_TRAILING_ONES(val_)  __builtin_ctzl(~(val_)) // number of trailing 1-bits, or undefined if there are none
#define NUM_TRAILING_ZEROS(val_) __builtin_ctzl(val_)    // number of trailing 0-bits, or undefined if there are none

C3D_FVec C3D_FVUnif[2][C3D_FVUNIF_COUNT];
C3D_IVec C3D_IVUnif[2][C3D_IVUNIF_COUNT];
u16      C3D_BoolUnifs[2];

u32  C3D_FVUnifDirty[2][C3D_FVUNIF_DIRTY_ARRAY_LENGTH];
bool C3D_IVUnifDirty[2][C3D_IVUNIF_COUNT];
bool C3D_BoolUnifsDirty[2];

static struct
{
	bool dirty;
	int count;
	float24Uniform_s* data;
} C3Di_ShaderFVecData[2];

static bool C3Di_IVUnifEverDirty[2][C3D_IVUNIF_COUNT];

void C3D_UpdateUniforms(GPU_SHADER_TYPE type)
{
	int offset = type == GPU_GEOMETRY_SHADER ? (GPUREG_GSH_BOOLUNIFORM-GPUREG_VSH_BOOLUNIFORM) : 0;
	int i = 0;

	// Update FVec uniforms that come from shader constants
	if (C3Di_ShaderFVecData[type].dirty)
	{
		while (i < C3Di_ShaderFVecData[type].count)
		{
			float24Uniform_s* u = &C3Di_ShaderFVecData[type].data[i++];
			GPUCMD_AddIncrementalWrites_Auto(GPUREG_VSH_FLOATUNIFORM_CONFIG+offset, (u32*)u, 4);
			C3D_RegClean(C3D_FVUnifDirty[type], u->id, 1);
		}
		C3Di_ShaderFVecData[type].dirty = false;
		i = 0;
	}

	// Update FVec uniforms
	// WYATT_TODO add support for batching across word boundaries
	for (u32 word = 0; word < C3D_FVUNIF_DIRTY_ARRAY_LENGTH; word++) {
		u32 regs_this_word = 0;
		u32 bits = C3D_FVUnifDirty[type][word];
		while (bits) {
			if (bits & 0b1) // First bit is set: dirty reg
			{
				const u32 dirty_regs = (bits == ~0) ? 32 : NUM_TRAILING_ONES(bits); // passing ~0 ends up returning ~0
				bits >>= dirty_regs;
				
				const u32 first_dirty = (word * 32) + regs_this_word;
				const u32 next_clean = first_dirty + dirty_regs;
				
				GPUCMD_AddWrite(GPUREG_VSH_FLOATUNIFORM_CONFIG+offset, 0x80000000 | first_dirty);
				GPUCMD_AddWrites_Auto(GPUREG_VSH_FLOATUNIFORM_DATA+offset, (u32*) &C3D_FVUnif[type][first_dirty], (next_clean - first_dirty) * 4);
				regs_this_word += dirty_regs;
			}
			else // First bit is clear: clean reg
			{
				const u32 clean_regs = NUM_TRAILING_ZEROS(bits);
				bits >>= clean_regs;
				regs_this_word += clean_regs;
			}
		}
	}

	// Clear the dirty flags
	for (i = 0; i < C3D_FVUNIF_DIRTY_ARRAY_LENGTH; i++)
		C3D_FVUnifDirty[type][i] = 0;

	// Update IVec uniforms
	for (i = 0; i < C3D_IVUNIF_COUNT; i ++)
	{
		if (!C3D_IVUnifDirty[type][i]) continue;

		GPUCMD_AddWrite(GPUREG_VSH_INTUNIFORM_I0+offset+i, C3D_IVUnif[type][i]);
		C3D_IVUnifDirty[type][i] = false;
		C3Di_IVUnifEverDirty[type][i] = false;
	}

	// Update bool uniforms
	if (C3D_BoolUnifsDirty[type])
	{
		GPUCMD_AddWrite(GPUREG_VSH_BOOLUNIFORM+offset, 0x7FFF0000 | C3D_BoolUnifs[type]);
		C3D_BoolUnifsDirty[type] = false;
	}
}

void C3Di_DirtyUniforms(GPU_SHADER_TYPE type)
{
	int i;
	C3D_BoolUnifsDirty[type] = true;
	if (C3Di_ShaderFVecData[type].count)
		C3Di_ShaderFVecData[type].dirty = true;
	C3D_RegDirty(&C3D_FVUnifDirty[type][0], 0, C3D_FVUNIF_COUNT); // Optimizes equally to setting directly
	for (i = 0; i < C3D_IVUNIF_COUNT; i ++)
		C3D_IVUnifDirty[type][i] = C3D_IVUnifDirty[type][i] || C3Di_IVUnifEverDirty[type][i];
}

void C3Di_LoadShaderUniforms(shaderInstance_s* si)
{
	GPU_SHADER_TYPE type = si->dvle->type;
	if (si->boolUniformMask)
	{
		C3D_BoolUnifs[type] &= ~si->boolUniformMask;
		C3D_BoolUnifs[type] |= si->boolUniforms;
	}

	if (type == GPU_GEOMETRY_SHADER)
		C3D_BoolUnifs[type] &= ~BIT(15);
	C3D_BoolUnifsDirty[type] = true;

	if (si->intUniformMask)
	{
		int i;
		for (i = 0; i < 4; i ++)
		{
			if (si->intUniformMask & BIT(i))
			{
				C3D_IVUnif[type][i] = si->intUniforms[i];
				C3D_IVUnifDirty[type][i] = true;
			}
		}
	}
	C3Di_ShaderFVecData[type].dirty = true;
	C3Di_ShaderFVecData[type].count = si->numFloat24Uniforms;
	C3Di_ShaderFVecData[type].data = si->float24Uniforms;
}

void C3Di_ClearShaderUniforms(GPU_SHADER_TYPE type)
{
	C3Di_ShaderFVecData[type].dirty = false;
	C3Di_ShaderFVecData[type].count = 0;
	C3Di_ShaderFVecData[type].data = NULL;
}
