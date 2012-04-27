/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.h
 * Video routines
 ***************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <ogcsys.h>

void InitVideo ();
void StopGX();
void ResetVideo_Menu();
void Menu_Render();
void DrawSquare(float x, float y, float width, float height, const GXColor &color);

//u8* CreateTextureFromFrameBuffer( u8 format, u32 &bufferSize, u16 &width, u16 &height );

extern int screenheight;
extern int screenwidth;
extern u32 frameCount;
extern Mtx44 MainProjection;
extern Mtx GXmodelView2D;

#endif
