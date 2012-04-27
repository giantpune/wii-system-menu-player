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
#include "tooltip.h"

ToolTip::ToolTip()
	: layout( NULL ),
	  brlan( NULL ),
	  tbox( NULL )
{
}

ToolTip::~ToolTip()
{
	delete obj;
	delete layout;
	delete brlan;
}

bool ToolTip::Load( const u8* balloonAshData, u32 balloonAshSize )
{
	DELETE( obj );
	DELETE( layout );
	DELETE( brlan );
	tbox = NULL;
	loaded = false;
	if( !balloonAshData || !balloonAshSize )
	{
		return false;
	}

	U8Archive arc( balloonAshData, balloonAshSize );
	if( !( layout = LoadLayout( arc, "my_IplTopBalloon_a" ) ) )
	{
		return false;
	}
	if( !(tbox = layout->FindTextbox( "T_Balloon" ) ) )
	{
		return false;
	}
	if( !(brlan = LoadAnimation( arc, "my_IplTopBalloon_a_balloonInOut" ) ) )
	{
		return false;
	}
	//tbox->SetText( NULL );

	obj = new Object;
	obj->AddAnimation( brlan );
	obj->BindPane( layout->FindPane( "RootPane" ));
	obj->SetAnimation( "my_IplTopBalloon_a_balloonInOut", 0, -1, -1, true, true );
	obj->Start();

	loaded = true;
	return true;
}

void ToolTip::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}
	layout->Render( modelview, ScreenProps, widescreen );
	obj->Advance();

}
