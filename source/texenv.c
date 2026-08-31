#include "internal.h"

C3D_TexEnv* C3D_GetTexEnv(int id)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return NULL;

	ctx->flags |= C3DiF_TexEnv(id);
	return &ctx->texEnv[id];
}

void C3D_SetTexEnv(int id, C3D_TexEnv* env)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	ctx->flags |= C3DiF_TexEnv(id);
	if (env)
		memcpy(&ctx->texEnv[id], env, sizeof(*env));
	else
		C3D_TexEnvInit(&ctx->texEnv[id]);
}

void C3D_DirtyTexEnv(C3D_TexEnv* env)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	u32 id = env-ctx->texEnv;
	if (id < 6)
		ctx->flags |= C3DiF_TexEnv(id);
}

void C3D_TexEnvBufUpdate(int mode, int mask)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	C3D_SetTexEnvBufUpdate(&ctx->texEnvBuf, mode, mask);
	ctx->flags |= C3DiF_TexEnvBuf;
}

void C3D_TexEnvBufColor(u32 color)
{
	C3D_Context* ctx = C3Di_GetContext();

	if (!(ctx->flags & C3DiF_Active))
		return;

	C3D_SetTexEnvBufColor(&ctx->texEnvBufClr, color);
	ctx->flags |= C3DiF_TexEnvBuf;
}
