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
#include "healthscreen.h"
#include "Inputs.h"
#include "utils/sc.h"

HealthScreen::HealthScreen()
	: healthLyt( NULL ),
	  backMenuLyt( NULL ),
	  healthObj( NULL ),
	  backMenuObj( NULL )
{
}
HealthScreen::~HealthScreen()
{
	delete healthObj;
	delete backMenuObj;
	delete healthLyt;
	delete backMenuLyt;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

bool HealthScreen::Load( const u8* healthAshData, u32 healthAshSize, const u8 *backMenuData, u32 backMenuSize )
{
	DELETE( healthObj );
	DELETE( backMenuObj );
	DELETE( healthLyt );
	DELETE( backMenuLyt );

	loaded = false;
	if( !healthAshData || !backMenuData )
	{
		return false;
	}
	U8Archive healthArc( healthAshData, healthAshSize );
	if( !( healthLyt = LoadLayout( healthArc, "it_Has_a" ) ) )
	{
		return false;
	}

	U8Archive backMenuArc( backMenuData, backMenuSize );
	if( !( backMenuLyt = LoadLayout( backMenuArc, "my_BackToWiiMenu" ) ) )
	{
		return false;
	}
#define LOADANIM( arc, x )									\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc, x ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ x ] = anim;									\
	}														\
	while( 0 )

	// load animations
	Animation *anim;
	LOADANIM( healthArc, "it_Has_a_Push" );
	LOADANIM( healthArc, "it_Has_a_SeenIn" );
	LOADANIM( healthArc, "it_Has_a_SeenOut" );
	LOADANIM( backMenuArc, "my_BackToWiiMenu" );

	healthObj = new Object;
	healthObj->BindPane( healthLyt->FindPane( "RootPane" ) );
	healthObj->AddAnimation( brlans.find( "it_Has_a_Push" )->second );
	healthObj->AddAnimation( brlans.find( "it_Has_a_SeenIn" )->second );
	healthObj->AddAnimation( brlans.find( "it_Has_a_SeenOut" )->second );
	healthObj->SetAnimation( "it_Has_a_SeenIn", 0.f, 30.f, -1, false );
	healthObj->Start();

	backMenuObj = new Object;
	backMenuObj->BindPane( backMenuLyt->FindPane( "RootPane" ) );
	backMenuObj->AddAnimation( brlans.find( "my_BackToWiiMenu" )->second );
	backMenuObj->SetAnimation( "my_BackToWiiMenu", 0, 140.f, -1.f, false );
	backMenuObj->Start();


	char pName[ 64 ];
	switch( CONF_GetRegion() )
	{
	case CONF_REGION_US: strcpy( pName, "US_" ); break;
	case CONF_REGION_EU: strcpy( pName, "EU_" ); break;
	default:// JPN, KOR, and CHN don't do this
		break;
	}
	strcat( pName, CONF_GetLanguageString() );


	Pane *pane1 = healthLyt->FindPane( "N_All" );

	if( !pane1 )
	{
		gprintf( "!pane1" );
		return false;
	}
	for( u32 i = 0; i < pane1->panes.size(); i++ )
	{
		Pane *pane = pane1->panes[ i ];
		// hide all the panes from other languages
		for( u32 j = 0; j < pane->panes.size(); j++ )
		{
			Pane *pane2 = pane->panes[ j ];
			if( !strstr( pane2->getName(), pName ) )
			{
				//gprintf( "disabled pane: %s\n", pane2->getName() );
				pane2->SetVisible( false );
			}
		}
	}

	// hide the "press A" until after the initial fade in
	SetPaneVisible( healthLyt, "N_Push", false );

	state = St_FadeIn;
	loaded = true;
	return true;
}

void HealthScreen::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}

	switch( state )
	{
	case St_IdlePressA:
	{
		healthLyt->Render( modelview, ScreenProps, widescreen );
		healthObj->Advance();
		// check for button presses
		for(int i = 3; i >= 0; i--)
		{
			if( Pad( i ).pA() || Pad( i ).hA() )
			{
				state = St_PressedA;
				healthObj->SetLoop( false );
				ClickedA();
				break;
			}
		}
	}
	break;
	case St_PressedA:
	case St_FadeIn:
	case St_FadeInDone:
	case St_FadeOutHealth:
		healthLyt->Render( modelview, ScreenProps, widescreen );
		healthObj->Advance();
		if( healthObj->IsFinished() )
		{
			UpdateState();
		}
		break;
	case St_Backmenu:
		backMenuLyt->Render( modelview, ScreenProps, widescreen );
		backMenuObj->Advance();
		if( backMenuObj->IsFinished() )
		{
			UpdateState();
		}
		break;
	default:
		return;
	}


}

void HealthScreen::UpdateState()
{
	switch( state )
	{
	case St_FadeIn:
		state = St_FadeInDone;
		break;
	case St_FadeInDone:
	{
		state = St_IdlePressA;
		healthObj->UnbindAllPanes();
		Pane *pane1;
		if( ( pane1 = healthLyt->FindPane( "N_Push" ) ) )
		{
			pane1->SetVisible( true );
			healthObj->BindPane( pane1 );
		}
		healthObj->SetAnimation( "it_Has_a_Push" );
		healthObj->Start();
	}
	break;
	case St_PressedA:
		healthObj->UnbindAllPanes();
		healthObj->BindPane( healthLyt->FindPane( "RootPane" ) );
		healthObj->SetAnimation( "it_Has_a_SeenOut", 0, 30, -1, false );
		healthObj->Start();
		state = St_FadeOutHealth;
	break;
	case St_FadeOutHealth:
		state = St_Backmenu;
	break;
	case St_Backmenu:
		state = St_Unk;
		Done();// emit a signal that this whole menu is done
		break;
	default:
		return;
	}
}
