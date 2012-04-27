#include <gccore.h>

// TODO replace those with something
extern GXRModeObj *vmode;

void GX_Project(f32 mx, f32 my, f32 mz, Mtx mv, const f32 *projection,
					const f32 *viewport, f32 *sx, f32 *sy, f32 *sz)
{
	float x, y, z, w;

	guVector vec = (guVector) { mx, my, mz };
	guVector vecRes;
	guVecMultiply(mv, &vec, &vecRes);

	if(projection[0] == GX_PERSPECTIVE)
	{
		x = (vecRes.x * projection[1]) + (vecRes.z * projection[2]);
		y = (vecRes.y * projection[3]) + (vecRes.z * projection[4]);
		z = (vecRes.z * projection[5]) + projection[6];
		w = -1.0f / vecRes.z;
	}
	else
	{
		x = (vecRes.x * projection[1]) + projection[2];
		y = (vecRes.y * projection[3]) + projection[4];
		z = (vecRes.z * projection[5]) + projection[6];
		w = 1.0f;
	}

	*sx = viewport[0] + (w * x * viewport[2] + viewport[2]) * 0.5f;
	*sy = viewport[1] - (w * y * viewport[3] - viewport[3]) * 0.5f;
	*sz = viewport[5] + (w * z * (viewport[5] - viewport[4]));
}

void GX_GetProjectionv(Mtx44 p, f32* ptr, u8 type)
{
	ptr[0] = (f32)type;
	ptr[1] = p[0][0];
	ptr[3] = p[1][1];
	ptr[5] = p[2][2];
	ptr[6] = p[2][3];

	if(type == GX_PERSPECTIVE)
	{
		ptr[2] = p[0][2];
		ptr[4] = p[1][2];
	}
	else
	{
		ptr[2] = p[0][3];
		ptr[4] = p[1][3];
	}
}

void GX_GetViewportv( f32* ptr )
{
	ptr[0] = 0.0f;
	ptr[1] = 0.0f;
	ptr[2] = vmode->fbWidth;
	ptr[3] = vmode->efbHeight;
	ptr[4] = 0.0f;
	ptr[5] = 1.0f;
}
