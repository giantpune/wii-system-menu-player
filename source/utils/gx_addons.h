#ifndef GX_ADDONS_H_
#define GX_ADDONS_H_

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

void GX_Project(f32 mx, f32 my, f32 mz, Mtx mv, const f32 *projection,
				const f32 *viewport, f32 *sx, f32 *sy, f32 *sz);
void GX_GetProjectionv(Mtx44 projection, f32* ptr, u8 type );
void GX_GetViewportv( f32* ptr );

#ifdef __cplusplus
}
#endif

#endif
