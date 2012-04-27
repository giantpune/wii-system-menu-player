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
#include "fatalerrordialog.h"

bool FatalErrordialog::Load( const u8* fatalDlgAshData, u32 fatalDlgAshSize )
{
	DELETE( layout );
	tbox = NULL;
	loaded = false;
	if( !fatalDlgAshData || !fatalDlgAshSize )
	{
		return false;
	}

	U8Archive fatalDlgArc( fatalDlgAshData, fatalDlgAshSize );
	if( !( layout = LoadLayout( fatalDlgArc, "my_Fatal" ) ) )
	{
		gprintf( "didn\'t find the fatal layout\n" );
		return false;
	}

	if( !(tbox = layout->FindTextbox( "TextBox_00" ) ) )
	{
		gprintf( "didn\'t find the fatal textbox\n" );
		return false;
	}
	tbox->SetText( NULL );
	loaded = true;
	return true;
}

void FatalErrordialog::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}
	// draw
	layout->Render( modelview, ScreenProps, widescreen );
}
