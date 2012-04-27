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
#include "bannerframe.h"
#include "bmg.h"
#include "buttoncoords.h"
#include "sc.h"

BannerFrame::BannerFrame()
	: channelFrame( NULL ),
	  leftBtn( NULL ),
	  rightBtn( NULL ),
	  state( St_Unk ),
	  clicked( -1 )
{
}

BannerFrame::~BannerFrame()
{
	delete leftBtn;
	delete rightBtn;
	delete channelFrame;
	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

bool BannerFrame::Load( const u8* chanTtlAshData, u32 chanTtlAshSize )
{
	if( loaded )
	{
		return true;
	}

	U8Archive chanTtlArc( chanTtlAshData, chanTtlAshSize );

	// probably don't need to load all these animations if we arent gonna use them
	if( !( channelFrame = LoadLayout( chanTtlArc, "my_ChTop_a" ) ) )
	{
		return false;
	}

	leftBtn = new QuadButton;
	rightBtn = new QuadButton;
#define LOADANIM( x )										\
	do														\
	{														\
		if( !(anim = LoadAnimation( chanTtlArc, x ) ) )		\
		{													\
			return false;									\
		}													\
		brlans[ x ] = anim;									\
		leftBtn->AddAnimation( anim );						\
		rightBtn->AddAnimation( anim );						\
	}														\
	while( 0 )

	// load animations
	Animation *anim;
	//LOADANIM( "my_ChTop_a_ChangeIn" );
	//LOADANIM( "my_ChTop_a_ChangeOut" );
	//LOADANIM( "my_ChTop_a_ChangeRoop" );
	//LOADANIM( "my_ChTop_a_ChangeTextIn" );	// maake the text in the buttons fade fast
	//LOADANIM( "my_ChTop_a_ChangeTextOut" );	// maake the text in the buttons fade fast
	LOADANIM( "my_ChTop_a_FocusBtnA_off" );	// make the buttons shrink to normal size
	LOADANIM( "my_ChTop_a_FocusBtn_on" );	// make the buttons grow
	//LOADANIM( "my_ChTop_a_OffBtn" );		// maake the text in the buttons fade a little slower
	//LOADANIM( "my_ChTop_a_OnBtn" );			// maake the text in the buttons fade a little slower
	LOADANIM( "my_ChTop_a_SelectBtn_Ac" );	// make the buttons flash and crap when they are clicked


	SetupButtons();
	/*leftBtnObj->BindPane( channelFrame->FindPane( "RootPane" ) );
	leftBtnObj->SetAnimation( "my_ChTop_a_SelectBtn_Ac" );
	leftBtnObj->Start();*/

	SetText( channelFrame, "T_BtnA", 1 );// "Wii Menu"
	SetText( channelFrame, "T_BtnB", 2 );// "Back"

	bool widescreen = ( _CONF_GetAspectRatio() > 0 );

	if( widescreen )
	{
		SetMaterialIndex( channelFrame, "Fre_a", 0, 0 );// change BannerMask_a_4x3.tpl to BannerMask_a_16x9.tpl
		SetMaterialIndex( channelFrame, "Fre_d", 0, 0 );
		SetMaterialIndex( channelFrame, "Fre_i", 0, 0 );
		SetMaterialIndex( channelFrame, "Fre_l", 0, 0 );

		SetMaterialIndex( channelFrame, "Fre_e", 0, 3 );// change BannerMask_b_4x3.tpl to BannerMask_b_16x9.tpl
		SetMaterialIndex( channelFrame, "Fre_g", 0, 3 );
		SetMaterialIndex( channelFrame, "Fre_f", 0, 3 );
		SetMaterialIndex( channelFrame, "Fre_h", 0, 3 );

		SetMaterialIndex( channelFrame, "Fre_b", 0, 5 );// change BannerMask_c_4x3.tpl to BannerMask_c_16x9.tpl
		SetMaterialIndex( channelFrame, "Fre_c", 0, 5 );
		SetMaterialIndex( channelFrame, "Fre_j", 0, 5 );
		SetMaterialIndex( channelFrame, "Fre_k", 0, 5 );
	}

	state = St_Idle;
	loaded = true;
	return true;
}

void BannerFrame::SetupButtons()
{
	u32 x, y, w, h;
	Animation *anim = brlans.find( "my_ChTop_a_FocusBtn_on" )->second;

	ButtonCoords( ChanTtl_L, x, y, w, h );
	leftBtn->SetCoords( x, y, w, h );
	ButtonCoords( ChanTtl_R, x, y, w, h );
	rightBtn->SetCoords( x, y, w, h );

	leftBtn->BindPane( channelFrame->FindPane( "N_BtnA" ) );		// left button
	rightBtn->BindPane( channelFrame->FindPane( "N_BtnB" ) );		// right button

	leftBtn->SetMouseOverAnimation( anim, 0, -1 );
	rightBtn->SetMouseOverAnimation( anim, 0, -1 );

	anim = brlans.find( "my_ChTop_a_FocusBtnA_off" )->second;
	leftBtn->SetMouseOutAnimation( anim, 0, -1 );
	rightBtn->SetMouseOutAnimation( anim, 0, -1 );

	anim = brlans.find( "my_ChTop_a_SelectBtn_Ac" )->second;
	leftBtn->SetClickAnimation( anim, 0, -1 );
	rightBtn->SetClickAnimation( anim, 0, -1 );

	leftBtn->SetTrigger( Button::Btn_A );
	rightBtn->SetTrigger( Button::Btn_A );

	leftBtn->Clicked.connect( this, &BannerFrame::LeftButtonClickedSlot );
	rightBtn->Clicked.connect( this, &BannerFrame::RightButtonClickedSlot );
	leftBtn->Finished.connect( this, &BannerFrame::LeftBtnAnimDone );
	rightBtn->Finished.connect( this, &BannerFrame::RightBtnAnimDone );
}

void BannerFrame::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen, u8 render_alpha )
{
	if( !loaded )
	{
		return;
	}

	Mtx mv;
	guMtxScaleApply( modelview, mv, ScreenProps.x / channelFrame->GetWidth(), 1.f, 1.0f );

	channelFrame->Render( modelview, ScreenProps, widescreen, render_alpha );

	// update to react to use input and advance to move to the next frame in the animation if there is one
	leftBtn->Update();
	leftBtn->Advance();
	rightBtn->Update();
	rightBtn->Advance();
}


void BannerFrame::ResetButtons()
{

	leftBtn->SetAnimation( "my_ChTop_a_FocusBtnA_off", 9, 10, 10, false );
	rightBtn->SetAnimation( "my_ChTop_a_FocusBtnA_off", 9, 10, 10, false );
	leftBtn->SetState( Button::St_Idle );
	rightBtn->SetState( Button::St_Idle );

	leftBtn->Start();
	rightBtn->Start();
}

void BannerFrame::LeftButtonClickedSlot()
{
	clicked = 0;
}

void BannerFrame::RightButtonClickedSlot()
{
	clicked = 1;
}

void BannerFrame::LeftBtnAnimDone()
{
	if( clicked == 0 )
	{
		clicked = -1;
		LeftBtnClicked();
	}
}

void BannerFrame::RightBtnAnimDone()
{
	if( clicked == 1 )
	{
		clicked = -1;
		RightBtnClicked();
	}
}
