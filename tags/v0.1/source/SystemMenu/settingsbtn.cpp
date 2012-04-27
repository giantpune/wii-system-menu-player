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
#include "bmg.h"
#include "buttoncoords.h"
#include "settingsbtn.h"

SettingsBtn::SettingsBtn()
	: layout( NULL ),
	  btn( NULL ),
	  wiiObj( new Object ),
	  cmnObj( new Object ),
	  clicked( false )
{
}

SettingsBtn::~SettingsBtn()
{
	delete cmnObj;
	delete wiiObj;
	delete btn;
	delete layout;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

bool SettingsBtn::Load( const u8* setupBtnAshData, u32 setupBtnAshSize )
{
	if( loaded )
	{
		return true;
	}

	if( !setupBtnAshData || !setupBtnAshSize )
	{
		return false;
	}

	U8Archive arc( setupBtnAshData, setupBtnAshSize );

	// create layout
	if( !(layout = LoadLayout( arc, "it_Button_a" ) ) )
	{
		return false;
	}
	u32 x, y, w, h;

	// create objects and buttons
	ButtonCoords( Setup_Back, x, y, w, h );
	btn = new QuadButton( x, y, w, h );

	// load animations
	Animation *anim;
#define LOADANIM( x, y )									\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc, y ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ y ] = anim;									\
		x->AddAnimation( anim );							\
	}														\
	while( 0 )

	LOADANIM( cmnObj, "it_Button_a_AlphIn" );
	LOADANIM( cmnObj, "it_Button_a_AlphOut" );
	LOADANIM( btn, "it_Button_a_BtnFlash" );
	LOADANIM( btn, "it_Button_a_BtnFoucusIn" );
	LOADANIM( btn, "it_Button_a_BtnFoucusOut" );
	LOADANIM( cmnObj, "it_Button_a_SeenIn" );
	LOADANIM( wiiObj, "it_Button_a_WiiAppear" );
	LOADANIM( wiiObj, "it_Button_a_WiiLost" );

	// assign panes to objects
	cmnObj->BindGroup( layout->FindGroup( "G_BarIn" ) );
	btn->BindGroup( layout->FindGroup( "G_SelectBtnA" ) );
	wiiObj->BindGroup( layout->FindGroup( "G_Wii" ) );

	// assign animations to objects
	cmnObj->SetAnimation( "it_Button_a_SeenIn", 0, -1, -1, false );
	cmnObj->Start();

	btn->SetMouseOverAnimation( brlans.find( "it_Button_a_BtnFoucusIn" )->second, 0, -1 );
	btn->SetMouseOutAnimation( brlans.find( "it_Button_a_BtnFoucusOut" )->second, 0, -1 );
	btn->SetClickAnimation( brlans.find( "it_Button_a_BtnFlash" )->second, 0, -1 );
	btn->SetTrigger( Button::Btn_A );

	btn->Clicked.connect( this, &SettingsBtn::BackBtnClicked );
	btn->Finished.connect( this, &SettingsBtn::BackAnimDone );

	SetText( layout, "T_Button_00", 252 );// "Back"

	loaded = true;
	return true;
}

void SettingsBtn::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}
	layout->Render( modelview, ScreenProps, widescreen );

	wiiObj->Advance();
	cmnObj->Advance();

	btn->Update();
	btn->Advance();

}

void SettingsBtn::DisableBackBtn( bool disable )
{
	//gprintf( "SettingsBtn::DisableBackBtn( %u )\n", disable );
	btn->SetEnabled( !disable );
}

void SettingsBtn::BackAnimDone()
{
	if( clicked )
	{
		clicked = false;
		BackClicked();
	}
}

void SettingsBtn::BackBtnClicked()
{
	clicked = true;
}

void SettingsBtn::HideWiiLogo( bool hide )
{
	if( hide )
	{
		wiiObj->SetAnimation( "it_Button_a_WiiLost", 0, -1, -1, false );
	}
	else
	{
		wiiObj->SetAnimation( "it_Button_a_WiiAppear", 0, -1, -1, false );
	}
	wiiObj->Start();
}
