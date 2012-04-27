/***************************************************************************
 * Copyright (C) 2009
 * by Tantric
 *
 * Additions and modifications by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include "utils/ImageWrite.h"
#include "utils/TextureConverter.h"

#define DEFAULT_FIFO_SIZE 256 * 1024 * 3
static unsigned int *xfb[2] = { NULL, NULL }; // Double buffered
static int whichfb = 0; // Switch
GXRModeObj *vmode = NULL; // Menu video mode
static unsigned char gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
Mtx44 MainProjection;
Mtx GXmodelView2D;
int screenwidth = 640;
int screenheight = 480;
u32 frameCount = 0;


/****************************************************************************
 * ResetVideo_Menu
 *
 * Reset the video/rendering mode for the menu
****************************************************************************/
void
ResetVideo_Menu()
{
	screenwidth = vmode->fbWidth;
	screenheight = vmode->efbHeight;

	f32 yscale;
	u32 xfbHeight;

	VIDEO_Configure (vmode);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	// clears the bg to color and clears the z buffer
	GXColor background = {0, 0, 0, 255};
	GX_SetCopyClear (background, 0x00ffffff);

	yscale = GX_GetYScaleFactor(vmode->efbHeight,vmode->xfbHeight );
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetDispCopySrc(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(vmode->aa,vmode->sample_pattern,GX_TRUE,vmode->vfilter);
	GX_SetFieldMode(vmode->field_rendering,((vmode->viHeight==2*vmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	if (vmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
	GX_InvVtxCache ();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	for(u32 i = 0; i < 8; i++)
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0+i, GX_TEX_ST, GX_F32, 0);

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(GXmodelView2D);
	guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, 0.0F);
	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

	GX_SetViewport(0,0,vmode->fbWidth,vmode->efbHeight,0,1);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    GX_SetZMode(0, GX_ALWAYS, 0);
	GX_SetColorUpdate(GX_TRUE);
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetZCompLoc(0);

    //bool widescreen = (CONF_GetAspectRatio() == 1);
	bool PAL50 = (CONF_GetVideo() == CONF_VIDEO_PAL) && (CONF_GetEuRGB60() == 0);
	Vec2f ScreenProps;
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;

	// in the grid-mode, it looks perfect just like this on my screen
	float xDiff = 310.f;

	float yDiff = 236.0f;
	if(PAL50)
		yDiff = 220.0f;

	//! This works good for banners
    f32 chopXOffset = (ScreenProps.x * 0.5f - xDiff );
    f32 chopYOffset = (ScreenProps.y * 0.5f - yDiff );
    f32 chopX = (ScreenProps.x * 0.5f + xDiff );
    f32 chopY = (ScreenProps.y * 0.5f + yDiff );

	guOrtho(MainProjection,chopYOffset,chopY,chopXOffset,chopX,-1000,1000);
	GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);
}

/****************************************************************************
 * InitVideo
 *
 * This function MUST be called at startup.
 * - also sets up menu video mode
 ***************************************************************************/

void
InitVideo ()
{
	VIDEO_Init();
	vmode = VIDEO_GetPreferredMode(NULL); // get default video mode

    vmode->viWidth = CONF_GetAspectRatio() == CONF_ASPECT_16_9 ? 708 : 640;

	if ((CONF_GetVideo() == CONF_VIDEO_PAL) && (CONF_GetEuRGB60() == 0))
	{
		vmode->viXOrigin = (VI_MAX_WIDTH_PAL - vmode->viWidth) / 2;
	}
	else
	{
		// widouth this +2, i get a stripe on the right side of my screen
		vmode->viXOrigin = ((VI_MAX_WIDTH_NTSC - vmode->viWidth) / 2) + 2;
	}
/*
	// testing...
	//vmode = &TVEurgb60Hz480IntDf;		// 480i
	//vmode = &TVPal528IntDf;				// 576i

	// i have a 16x9 tv, but when i put the wii in 4x3, i got some black stripes on
	// the edges.  this seems to work out well for both modes, not just 16x9
	if ( 1 || CONF_GetAspectRatio() == CONF_ASPECT_16_9)
	{
		// widescreen fix
		vmode->viWidth = VI_MAX_WIDTH_PAL - 12;
		vmode->viXOrigin = ((VI_MAX_WIDTH_PAL - vmode->viWidth) / 2) + 2;
	}
*/
	// Allocate the video buffers
	xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
	xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));

	VIDEO_Configure (vmode);

	// Clear framebuffers etc.
	VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);
	VIDEO_SetNextFramebuffer (xfb[0]);

	VIDEO_SetBlack (FALSE);
	VIDEO_Flush ();
	VIDEO_WaitVSync ();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync ();

	// Initialize GX
	GXColor background = { 0, 0, 0, 0xff };
	memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);
	GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear (background, 0x00ffffff);
	GX_SetDispCopyGamma (GX_GM_1_0);
	GX_SetCullMode (GX_CULL_NONE);

	ResetVideo_Menu();
	// Finally, the video is up and ready for use :)
}

/****************************************************************************
 * StopGX
 *
 * Stops GX (when exiting)
 ***************************************************************************/
void StopGX()
{
	GX_DrawDone();
	GX_AbortFrame();
	GX_Flush();

	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
}

/****************************************************************************
 * Menu_Render
 *
 * Renders everything current sent to GX, and flushes video
 ***************************************************************************/
void Menu_Render()
{
	whichfb ^= 1; // flip framebuffer
	GX_CopyDisp(xfb[whichfb],GX_TRUE);
	GX_DrawDone();
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	frameCount++;
}

/****************************************************************************
 * PrintScreenshot
 ***************************************************************************/
void PrintScreenshot()
{
	char path[200];
	time_t t = time(0);
	sprintf(path, "sd:/%i.png", (u32) t);

	gprintf("\nTakeScreenshot(%s)", path);
	int size = (2 * vmode->fbWidth * vmode->xfbHeight + 31) & ~31;

	u8 * buffer = (u8 *) memalign(32, size);
	if(!buffer)
		return;

	memcpy(buffer, xfb[whichfb], size);

	gdImagePtr gdImg;
	if(YCbYCrToGD(buffer, vmode->fbWidth, vmode->xfbHeight, &gdImg))
	{
		WriteGDImage(path,gdImg, IMAGE_PNG, 0);
		gdImageDestroy(gdImg);
	}
	free(buffer);

	return;
}

/****************************************************************************
 * Simple filled square in a color
 ***************************************************************************/
void DrawSquare(float x, float y, float width, float height, const GXColor &color)
{
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    // channel control
    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0,GX_DISABLE,GX_SRC_REG,GX_SRC_VTX,GX_LIGHTNULL,GX_DF_NONE,GX_AF_NONE);

    // texture gen.
    GX_SetNumTexGens(1);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    // texture environment
    GX_SetNumTevStages(1);
    GX_SetNumIndStages(0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1_4);
    GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GX_SetTevDirect(GX_TEVSTAGE0);
    // swap table
    GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GX_SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GX_SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
    // alpha compare and blend mode
    GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    GX_ClearVtxDesc();
    GX_InvVtxCache();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    f32 x2 = x + width;
	f32 y2 = y + height;
	guVector v[] = { { x, y, 0.0f }, { x2, y, 0.0f }, { x2, y2, 0.0f }, { x, y2, 0.0f }, { x, y, 0.0f } };

    GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
    for (int i = 0; i < 4; i++)
    {
        GX_Position3f32(v[i].x, v[i].y, v[i].z);
        GX_Color4u8(color.r, color.g, color.b, color.a);
    }
    GX_End();
}

# if 0
u8* CreateTextureFromFrameBuffer( u8 format, u32 &bufferSize, u16 &width, u16 &height )
{
	u32 len = GX_GetTexBufferSize( vmode->fbWidth, vmode->xfbHeight, format, GX_FALSE, 0 );
	u8* outbuf = (u8*)memalign( 32, len );
	if( !outbuf )
	{
		return NULL;
	}

	GX_SetTexCopySrc( 0, 0, vmode->fbWidth, vmode->xfbHeight );
	GX_SetTexCopyDst( vmode->fbWidth, vmode->xfbHeight, format, GX_FALSE );

	GX_CopyTex( outbuf, GX_FALSE );
	GX_PixModeSync();

	bufferSize = len;

	width = vmode->fbWidth;
	height = vmode->xfbHeight;

	return outbuf;
}
#endif
