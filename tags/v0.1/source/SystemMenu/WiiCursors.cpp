/*
Copyright (c) 2012 - Dimok and giantpune

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include <wiiuse/wpad.h>
#include "WiiCursors.h"
#include "Inputs.h"
#include "video.h"

static const char *cursorLyt[4] =
{
	"P1_Def",
	"P2_Def",
	"P3_Def",
	"P4_Def"
};

bool WiiCursors::bInitialized = false;
Mtx44 WiiCursors::projection;

WiiCursors::WiiCursors()
{
    // copy initial projection as standard for all cursors
    if(!bInitialized) {
        bInitialized = true;
        memcpy(projection, MainProjection, sizeof(Mtx44));
    }

	for(int i = 0; i < 4; i++)
	{
		cursor[i] = NULL;
		transPane[ i ] = NULL;
		rotPane[ i ] = NULL;
		rotSPane[ i ] = NULL;
	}
}

WiiCursors::~WiiCursors()
{
	for(int i = 0; i < 4; i++)
	{
		delete cursor[i];
	}
}

bool WiiCursors::Load( const u8* cursorAshData, u32 cursorAshSize )
{
	U8Archive cursorArc( cursorAshData, cursorAshSize );

	for(int i = 0; i < 4; i++)
	{
		// probably don't need to load all these animations if we arent gonna use them
		if( !( cursor[i] = LoadLayout( cursorArc, cursorLyt[i] ) ) )
			return false;
		if( !(transPane[ i ] = cursor[i]->FindPane( "N_Trans" )) )
		{
			return false;
		}
		if( !(rotSPane[ i ] = cursor[i]->FindPane( "N_SRot" )) )
		{
			return false;
		}
		if( !(rotPane[ i ] = cursor[i]->FindPane( "N_Rot" )) )
		{
			return false;
		}
	}

	loaded = true;
	return true;
}

float lastX = 0, lastY = 0;
void WiiCursors::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
		return;

    // WiiCursor has own projection which is never affected by zooming or other effects
    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

	for(int i = 3; i >= 0; i--)
	{
		const WPADData &wpad = Pad(i).GetData();
		if(wpad.ir.valid)
		{
			float posX = -0.5f * ScreenProps.x + wpad.ir.x;
			float posY = 0.5f * ScreenProps.y - wpad.ir.y;

			if( ( i == 0 ) && ( lastX != wpad.ir.x || lastY != wpad.ir.y ) )
			{
				//gprintf( "pos: %.0f, %.0f\n", wpad.ir.x, wpad.ir.y );
				lastX = wpad.ir.x;
				lastY = wpad.ir.y;
			}

			transPane[ i ]->SetPosition( posX, posY );
			rotPane[ i ]->SetRotate( -wpad.ir.angle );
			rotSPane[ i ]->SetRotate( -wpad.ir.angle );
			cursor[ i ]->Render( modelview, ScreenProps, widescreen );
		}
	}

    // load original projection again
    GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);
}
