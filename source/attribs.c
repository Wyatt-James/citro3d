#include "internal.h"
#include <c3d/attribs.h>

void AttrInfo_Init(C3D_AttrInfo* info)
{
	memset(info, 0, sizeof(*info));
	info->flags[1] = 0xFFF << 16;
}

int AttrInfo_AddLoader(C3D_AttrInfo* info, int regId, GPU_FORMATS format, int count)
{
	if (info->attrCount == 12) return -1;
	int id = info->attrCount++;
	if (regId < 0) regId = id;
	if (id < 8)
		info->flags[0] |= GPU_ATTRIBFMT(id, count, format);
	else
		info->flags[1] |= GPU_ATTRIBFMT(id-8, count, format);

	info->flags[1] = (info->flags[1] &~ (0xF0000000 | BIT(id+16))) | (id << 28);
	info->permutation |= regId << (id*4);
	return id;
}

int AttrInfo_AddFixed(C3D_AttrInfo* info, int regId)
{
	if (info->attrCount == 12) return -1;
	int id = info->attrCount++;
	if (regId < 0) regId = id;

	info->flags[1] = (info->flags[1] &~ 0xF0000000) | (id << 28);
	info->permutation |= regId << (id*4);
	return id;
}

C3D_AttrInfo* C3D_GetAttrInfo(void)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return NULL;

	ctx->flags |= C3DiF_AttrInfo;
	return &ctx->attrInfo;
}

void C3D_SetAttrInfo(C3D_AttrInfo* info)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	if (info != &ctx->attrInfo)
		memcpy(&ctx->attrInfo, info, sizeof(*info));
	ctx->flags |= C3DiF_AttrInfo;
}





















// // Don't use me. Use the macros instead.
// static inline void f1(u32 header, const u32* param, u32 paramlength)
// {
// 	if(GPUCMD_UNLIKELY(!gpuCmdBuf || gpuCmdBufOffset+paramlength+1>gpuCmdBufSize)) {
// 		GPUCMD_SvcBreakUserPanicWrapper(); // Shouldn't happen.
// 		// return;
// 	}

// 	paramlength--;
// 	header|=(paramlength&0xff)<<20;

// 	gpuCmdBuf[gpuCmdBufOffset++]=param ? param[0] : 0;
// 	gpuCmdBuf[gpuCmdBufOffset++]=header;

// 	if(GPUCMD_LIKELY(paramlength))
// 	{
// 		if(GPUCMD_LIKELY(param))
// 		{
// 			for (int i = 0; i < paramlength; i++) {
// 				gpuCmdBuf[gpuCmdBufOffset + i] = param[1 + i];
// 			}
// 		}
// 		else
// 		{
// 			for (int i = 0; i < paramlength; i++) {
// 				gpuCmdBuf[gpuCmdBufOffset + i] = 0;
// 			}
// 		}
// 	}

// 	gpuCmdBufOffset+=paramlength + (paramlength & 1); // Add LSB twice for alignment
// 	// if(paramlength&1)gpuCmdBuf[gpuCmdBufOffset++]=0x00000000; //alignment
// }

// // Don't use me. Use the macros instead.
// static inline void f2(u32 header, const u32* param, u32 paramlength)
// {
// 	if(!paramlength)paramlength=1;

// 	while(paramlength)
// 	{
// 		u32 remaining = paramlength > 0x100 ? 0x100 : paramlength;
// 		f1(header, param, remaining);
// 		param += remaining;
// 		paramlength -= remaining;
// 		if(header & BIT(31)) header += remaining;
// 	}
// }

// #define minline(reg, mask, vals, num) f2(GPUCMD_HEADER(1, (mask), (reg)), (vals), (num))

// /// Adds multiple masked incremental register writes to the current command buffer.
// /// This "auto" macro will attempt to automatically inline calls
// /// where "num" is a constant expression, and also <= a threshold.
// #define mmask(reg, mask, vals, num)			
// do {																			
// 	if (GPUCMD_IS_CONSTEXPR(num) && (num) <= GPUCMD_INLINE_THRESH)				
// 		minline((reg), (mask), (vals), (num));	
// 	else																		
// 		GPUCMD_AddMaskedIncrementalWrites((reg), (mask), (vals), (num));		
// } while(0)

// /// Adds multiple incremental register writes to the current command buffer.
// /// This "auto" macro will attempt to automatically inline calls
// /// where "num" is a constant expression, and also <= a threshold.
// #define mnomask(reg, vals, num) mmask((reg), 0xF, (vals), (num))

void C3Di_AttrInfoBind(C3D_AttrInfo* info)
{
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_ATTRIBBUFFERS_FORMAT_LOW, (u32*)info->flags, sizeof(info->flags)/sizeof(u32));
	gpucmd_single_t cmds[] = {
		GPUCMD_MaskedSingle(GPUREG_VSH_INPUTBUFFER_CONFIG, 0xB, 0xA0000000 | (info->attrCount - 1)),
		GPUCMD_Single(GPUREG_VSH_NUM_ATTR, info->attrCount - 1),
	};
	GPUCMD_AddBatchOfSingles(cmds);
	GPUCMD_AddIncrementalWrites_Auto(GPUREG_VSH_ATTRIBUTES_PERMUTATION_LOW, (u32*)&info->permutation, 2);
}
