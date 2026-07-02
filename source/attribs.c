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
