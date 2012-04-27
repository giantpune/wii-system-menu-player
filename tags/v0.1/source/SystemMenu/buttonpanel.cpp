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
#include "buttoncoords.h"
#include "buttonpanel.h"

ButtonPanel::ButtonPanel()
	: layout( NULL ),
	  leftArrow( NULL ),
	  rightArrow( NULL ),
	  settingsBtn( NULL ),
	  mailBtn( NULL ),
	  clicked( -1 )
{
}

ButtonPanel::~ButtonPanel()
{
	delete leftArrow;
	delete rightArrow;
	delete settingsBtn;
	delete mailBtn;

	std::map< std::string, Animation *>::iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}

	delete layout;
}

bool ButtonPanel::Load( const u8* cmnBtnAshData, u32 cmnBtnAshSize )
{
	if( loaded )
	{
		return true;
	}
	U8Archive cmnBtnArc( cmnBtnAshData, cmnBtnAshSize );

	if( !( layout = LoadLayout( cmnBtnArc, "my_IplTop_e" ) ) )
	{
		return false;
	}

	// these 2 panes show up on the "mail" button when it wants to tell you you have some messages
	SetPaneVisible( layout, "T_BbsMark1", false );
	SetPaneVisible( layout, "Picture_00", false );

	Animation *my_IplTop_e = LoadAnimation( cmnBtnArc, "my_IplTop_e" );
	if( !my_IplTop_e )
	{
		return false;
	}

	brlans[ "my_IplTop_e" ] = my_IplTop_e;
	if( !CreateButtons( cmnBtnArc ) )
	{
		return false;
	}
	/*testObject->BindPane( layout->FindPane( "RootPane" ) );
	//testObject->BindMaterials( layout->Materials() );
	testObject->BindMaterial( layout->FindMaterial( "ArwBtnR" ) );// right arrow "+" sign
	testObject->BindMaterial( layout->FindMaterial( "ArwBtnR_Ac" ) );// right arrow "+" sign click
	//testObject->BindPane( layout->FindPane( "N_ArwR_Roop" ) );// right arrow
	testObject->BindMaterial( layout->FindMaterial( "ArwBtnL" ) );// left arrow "-" sign
	testObject->BindMaterial( layout->FindMaterial( "ArwBtnL_Ac" ) );// left arrow "-" sign
	//testObject->BindPane( layout->FindPane( "N_ArwL_Roop" ) );// left arrow


	//testObject->BindPane( layout->FindPane( "N_ArwL_Roop" ) );// left arrow
	//testObject->BindPane( layout->FindPane( "N_ArwBtnR" ) );// left arrow
	//testObject->BindPane( layout->FindPane( "N_BtnR_a0_Bbs" ) );// mail button



	//testObject->BindGroup( layout->FindGroup( "N_ArwR_Roop" ) );


	testObject->AddAnimation( my_IplTop_e );
	//testObject->SetAnimation( "my_IplTop_e", 10011.0f, 10053.0f, -1.f, true, true );// arrow idle animation

	//testObject->SetAnimation( "my_IplTop_e", 10600, 10620, -1.f, true );// arrow idle animation
	testObject->SetAnimation( "my_IplTop_e", 10700, 10720, -1.f, true );// arrow clicked anim


	//testObject->SetAnimation( "my_IplTop_e", 900, 910 );// mailbox btn over animation
	testObject->Start();*/
	state = St_IdleGrid;
	loaded = true;

	return true;
}

bool ButtonPanel::CreateButtons( const U8Archive &arc )
{
	u32 x, y, w, h;
	std::map< std::string, Animation *>::iterator it = brlans.find( "my_IplTop_e" );
	Animation *anim = it->second;

	ButtonCoords( CmnBtn_Mail, x, y, w, h );
	mailBtn = new CircleButton( x, y, w );
	mailBtn->AddAnimation( anim );
	mailBtn->BindPane( layout->FindPane( "N_BtnR_a0_Bbs" ) );// mailbox button
	mailBtn->SetIdleAnimation( anim, 899, 900 );
	mailBtn->SetMouseOverAnimation( anim, 900, 910 );
	mailBtn->SetMouseOutAnimation( anim, 930, 939 );

	ButtonCoords( CmnBtn_Settings, x, y, w, h );
	settingsBtn = new CircleButton( x, y, w );
	settingsBtn->AddAnimation( anim );
	settingsBtn->BindPane( layout->FindPane( "N_BtnL_a0_Set" ) );// settings button
	settingsBtn->SetClickAnimation( anim, 6990, 7013 );
	settingsBtn->SetIdleAnimation( anim, 6989, 6900 );
	settingsBtn->SetMouseOverAnimation( anim, 6900, 6910 );
	settingsBtn->SetMouseOutAnimation( anim, 6930, 6939 );
	settingsBtn->SetTrigger( Button::Btn_A );

	ButtonCoords( CmnBtn_LeftArrowSM, x, y, w, h );
	leftArrow = new QuadButton( x, y, w, h );
	leftArrow->AddAnimation( anim );
	leftArrow->BindPane( layout->FindPane( "N_ArwL_Roop" ) );// left arrow, loop
	leftArrow->BindPane( layout->FindPane( "N_ArwL_End" ) );// left arrow, end
	leftArrow->BindMaterial( layout->FindMaterial( "ArwBtnL" ) );// left arrow "-" sign
	leftArrow->BindMaterial( layout->FindMaterial( "ArwBtnL_Ac" ) );// left arrow "-" sign, clicked
	leftArrow->SetAnimation( "my_IplTop_e", 10011.0f, 10053.0f, -1.f, true, true );
	leftArrow->SetIdleAnimation( anim, 10011.0f, 10053.0f, true, true );
	leftArrow->SetMouseOverAnimation( anim, 10600, 10620 );
	leftArrow->SetMouseOutAnimation( anim, 10800, 10812 );
	leftArrow->SetClickAnimation( anim, 10700, 10720 );
	leftArrow->SetTrigger( Button::Btn_A );
	leftArrow->Start();

	ButtonCoords( CmnBtn_RightArrowSM, x, y, w, h );
	rightArrow = new QuadButton( x, y, w, h );
	rightArrow->AddAnimation( anim );
	rightArrow->BindPane( layout->FindPane( "N_ArwR_Roop" ) );// right arrow, loop
	rightArrow->BindPane( layout->FindPane( "N_ArwR__End" ) );// right arrow, end
	rightArrow->BindMaterial( layout->FindMaterial( "ArwBtnR" ) );// right arrow "+" sign
	rightArrow->BindMaterial( layout->FindMaterial( "ArwBtnR_Ac" ) );// right arrow "+" sign, clicked
	rightArrow->SetAnimation( "my_IplTop_e", 10011.0f, 10053.0f, -1.f, true, true );
	rightArrow->SetIdleAnimation( anim, 10011.0f, 10053.0f, true, true );
	rightArrow->SetMouseOverAnimation( anim, 10600, 10620 );
	rightArrow->SetMouseOutAnimation( anim, 10700, 10712 );
	rightArrow->SetClickAnimation( anim, 10700, 10720 );
	rightArrow->SetTrigger( Button::Btn_A );
	rightArrow->Start();

	leftArrow->MouseOver.connect( this, &ButtonPanel::LeftArrowMouseOver );
	leftArrow->MouseOut.connect( this, &ButtonPanel::LeftArrowMouseOut );
	rightArrow->MouseOver.connect( this, &ButtonPanel::RightArrowMouseOver );
	rightArrow->MouseOut.connect( this, &ButtonPanel::RightArrowMouseOut );
	leftArrow->AnimationStarted.connect( this, &ButtonPanel::LeftArrowAnimStarted );
	rightArrow->AnimationStarted.connect( this, &ButtonPanel::RightArrowAnimStarted );

	settingsBtn->Clicked.connect( this, &ButtonPanel::SettinsgBtnClicked );
	settingsBtn->Finished.connect( this, &ButtonPanel::SettinsgBtnAnimDone );


	return true;
}

void ButtonPanel::ShowArrowsOnly( bool hideEverythingElse )
{
	SetPaneVisible( layout, "N_BtnR_a0", !hideEverythingElse );
	SetPaneVisible( layout, "N_BtnR_a1", !hideEverythingElse );

	SetPaneVisible( layout, "N_BtnL_a0", !hideEverythingElse );
	SetPaneVisible( layout, "N_BtnL_a1", !hideEverythingElse );

	settingsBtn->SetEnabled( !hideEverythingElse );
	mailBtn->SetEnabled( !hideEverythingElse );
}

void ButtonPanel::LeftArrowMouseOver()
{
	u32 x, y, w, h;
	ButtonCoords( CmnBtn_LeftArrowLG, x, y, w, h );
	leftArrow->SetCoords( x, y, w, h );
}

void ButtonPanel::LeftArrowMouseOut()
{
	u32 x, y, w, h;
	ButtonCoords( CmnBtn_LeftArrowSM, x, y, w, h );
	leftArrow->SetCoords( x, y, w, h );
}

void ButtonPanel::RightArrowMouseOver()
{
	u32 x, y, w, h;
	ButtonCoords( CmnBtn_RightArrowLG, x, y, w, h );
	rightArrow->SetCoords( x, y, w, h );
}

void ButtonPanel::RightArrowMouseOut()
{
	u32 x, y, w, h;
	ButtonCoords( CmnBtn_RightArrowSM, x, y, w, h );
	rightArrow->SetCoords( x, y, w, h );
}

void ButtonPanel::LeftArrowAnimStarted( AnimationLink *anim )
{
	if( anim->currentFrame == 10011
			&& !rightArrow->IsFinished()
			&& rightArrow->GetFrame() >= 10011
			&& rightArrow->GetFrame() <= 10053 )
	{
		leftArrow->SetFrame( rightArrow->GetFrame() );
	}
}

void ButtonPanel::RightArrowAnimStarted( AnimationLink *anim )
{
	if( anim->currentFrame == 10011
			&& !leftArrow->IsFinished()
			&& leftArrow->GetFrame() >= 10011
			&& leftArrow->GetFrame() <= 10053 )
	{
		rightArrow->SetFrame( leftArrow->GetFrame() );
	}
}

void ButtonPanel::HideLeftArrow( bool hide )
{
	if( hide )
	{
		leftArrow->SetAnimation( "my_IplTop_e", 10100, 10110, -1, false );
		leftArrow->SetEnabled( false );
		leftArrow->SetState( Button::St_Idle );
		leftArrow->Start();
	}
	else
	{
		leftArrow->SetAnimation( "my_IplTop_e", 10150, 10160 );
		leftArrow->ScheduleAnimation( "my_IplTop_e", 10011.0f, 10053.0f, -1.f, true, true );
		leftArrow->SetEnabled( true );
		leftArrow->Start();
	}
}

void ButtonPanel::HideRightArrow( bool hide )
{
	if( hide )
	{
		rightArrow->SetAnimation( "my_IplTop_e", 10100, 10110, -1, false );
		rightArrow->SetEnabled( false );
		rightArrow->SetState( Button::St_Idle );
		rightArrow->Start();
	}
	else
	{
		rightArrow->SetAnimation( "my_IplTop_e", 10150, 10160 );
		rightArrow->ScheduleAnimation( "my_IplTop_e", 10011.0f, 10053.0f, -1.f, true, true );
		rightArrow->SetEnabled( true );
		rightArrow->Start();
	}
}

void ButtonPanel::UpdateInput()
{
	mailBtn->Update();
	mailBtn->Advance();

	settingsBtn->Update();
	settingsBtn->Advance();

	leftArrow->Update();
	leftArrow->Advance();

	rightArrow->Update();
	rightArrow->Advance();
}

void ButtonPanel::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}

	Mtx mv;
	guMtxScaleApply( modelview, mv, ScreenProps.x / layout->GetWidth(), 1.f, 1.0f );

	layout->Render( modelview, ScreenProps, widescreen );
	//layout->Render( mv, ScreenProps, widescreen );
	//testObject->Advance();

	// update buttons
	//mailBtn->Update();
	//mailBtn->Advance();

	//settingsBtn->Update();
	//settingsBtn->Advance();

	//leftArrow->Update();
	//leftArrow->Advance();

	//rightArrow->Update();
	//rightArrow->Advance();
	//gprintf( "left arrow: %.0f\n", leftArrow->GetFrame() );
}

void ButtonPanel::SettinsgBtnClicked()
{
	clicked = 0;
}

void ButtonPanel::SettinsgBtnAnimDone()
{
	if( clicked == 0 )
	{
		clicked = -1;
		settingsBtn->SetFrame( 10011 );
		settingsBtn->SetState( Button::St_Idle );
		SettingsBtnClicked();
	}
}
