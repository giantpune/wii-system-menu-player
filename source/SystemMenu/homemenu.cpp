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
#include <stdio.h>

#include "buttoncoords.h"
#include "homemenu.h"
#include "sc.h"

HomeMenu::HomeMenu()
	: lyt( new Layout ),
	  btn1( NULL ),
	  btnTopBar( NULL ),
	  btnBottomBar( NULL ),
	  btnWiimote( NULL ),
	  btnDlg1( NULL ),
	  btnDlg2( NULL ),
	  testObj( new Object ),
	  dlgObj( new Object )
{
	dlgTxt[ 0 ] = 0;
	for( int i = 0; i < 4; i++ )
	{
		btryObj[ i ] = new Object;
		btryLevel[ i ] = -1;
	}
}

HomeMenu::~HomeMenu()
{
	delete btn1;
	delete btnTopBar;
	delete btnBottomBar;
	delete btnWiimote;
	delete btnDlg1;
	delete btnDlg2;
	delete btryObj[0];
	delete btryObj[1];
	delete btryObj[2];
	delete btryObj[3];

	delete dlgObj;
	delete testObj;

	delete lyt;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

void MakeAllVisible( Pane *pane )
{
	//bool changed = false;
	if( pane->GetAlpha() != 0xff || pane->GetHide() || !pane->GetVisible() )
	{
		gprintf( "changing pane: \"%s\" %02x, %u, %u \n", pane->getName(), pane->GetAlpha(), pane->GetHide(), pane->GetVisible() );
	}
	pane->SetAlpha( 0xff );
	pane->SetVisible( true );
	pane->SetHide( false );

	foreach( Pane *p, pane->panes )
	{
		MakeAllVisible( p );
	}
}

/*void HomeMenu::SetBGTexture( Texture *tex )
{
	const char* bgpane = "back_00";
	Pane *pane;
	Material *mat = lyt->FindMaterial( bgpane );
	if( !mat )
	{
		return;
	}
	mat->SetForcedTexture( tex );

	if( ( pane = lyt->FindPane( "back_02" ) ) )
	{
		pane->SetAlpha( tex ? 0 : 0xff );
	}
	if( ( pane = lyt->FindPane( bgpane ) ) )
	{
		pane->SetAlpha( tex ? 0xff : 0 );
	}
}*/

bool HomeMenu::Load( const u8* homeBtn1AshData, u32 homeBtn1AshSize, const u8* lang_homeBtn1AshData, u32 lang_homeBtn1AshSize )
{
	if( loaded )
	{
		return true;
	}

	if( !homeBtn1AshData || !homeBtn1AshSize || !lang_homeBtn1AshData || !lang_homeBtn1AshSize )
	{
		return false;
	}

	Pane *pane;
	U8Archive arc1( homeBtn1AshData, homeBtn1AshSize );
	U8Archive arc2( lang_homeBtn1AshData, lang_homeBtn1AshSize );

	// since the textures are stread out over 2 archives, load them manually
	// read layout data
	u8 *stuff = arc2.GetFile( "/arc/blyt/th_HomeBtn_d.brlyt" );
	if( !stuff )
	{
		return false;
	}

	// load layout
	if( !lyt->Load( stuff ) )
	{
		return false;
	}

	const char *lang = CONF_GetLanguageString();

	// load textures from an archive based on their name
	foreach( Texture *t, lyt->Textures() )
	{
		if( strstr( t->getName().c_str(), lang ) )
		{
			if( !LoadTpl( t, arc2 ) )
			{
				return false;
			}
		}
		else if( !LoadTpl( t, arc1 ) )
		{
			return false;
		}
	}

	SetPaneVisible( lyt, "let_icn_00", false );

	// just show the striped background for now
	if( ( pane = lyt->FindPane( "back_02" ) ) )
	{
		pane->SetAlpha( 0xff );
	}

	u32 x, y, w, h;
	ButtonCoords( HBMTopBar, x, y, w, h );
	btnTopBar = new QuadButton( x, y, w, h );

	ButtonCoords( HBMBottomBar, x, y, w, h );
	btnBottomBar = new QuadButton( x, y, w, h );

	ButtonCoords( HBMWiimoteDown, x, y, w, h );
	btnWiimote = new QuadButton( x, y, w, h );

	ButtonCoords( HBMCenterBtn, x, y, w, h );
	btn1 = new QuadButton( x, y, w, h );

	ButtonCoords( Dlg_A2_Btn1, x, y, w, h );
	btnDlg1 = new QuadButton( x, y, w, h );

	ButtonCoords( Dlg_A2_Btn2, x, y, w, h );
	btnDlg2 = new QuadButton( x, y, w, h );


	Animation* anim;

#define LOADANIM( x, y )									\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc1, y ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ y ] = anim;									\
		x->AddAnimation( anim );							\
	}														\
	while( 0 )

#define LOADANIM2( x, y, z )								\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc1, z ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ z ] = anim;									\
		x->AddAnimation( anim );							\
		y->AddAnimation( anim );							\
	}														\
	while( 0 )

#define LOADANIMBTRY( x )									\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc1, x ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ x ] = anim;									\
		btryObj[0]->AddAnimation( anim );					\
		btryObj[1]->AddAnimation( anim );					\
		btryObj[2]->AddAnimation( anim );					\
		btryObj[3]->AddAnimation( anim );					\
	}														\
	while( 0 )

	btnTopBar->BindPane( lyt->FindPane( "bar_00" ) );// top bar
	btnBottomBar->BindPane( lyt->FindPane( "bar_10" ) );// bottom bar
	//testObj->BindPane( lyt->FindPane( "back_02" ) );//	background with the horizontal lines
	btn1->BindPane( lyt->FindPane( "N_cntBtn_all" ) );// center "Wii Menu" button, it has the stupid letter image on it
	btnWiimote->BindPane( lyt->FindPane( "N_cntrl_00" ) );// the huge wiimote that slides in from the bottom of the page


#define BTRY_MAT( x )											\
	do															\
	{															\
		snprintf( name, sizeof( name ), x, i );					\
		btryObj[i]->BindMaterial( lyt->FindMaterial( name ) );	\
	}															\
	while( 0 )

	// setup batteries
	for( int i = 0; i < 4; i++ )
	{
		char name[ 20 ];
		BTRY_MAT( "btryCase_%.2u" );
		BTRY_MAT( "btryPwr_%.2u_0" );
		BTRY_MAT( "btryPwr_%.2u_1" );
		BTRY_MAT( "btryPwr_%.2u_2" );
		BTRY_MAT( "btryPwr_%.2u_3" );
		BTRY_MAT( "tx_plyr_%.2u" );
	}

	LOADANIMBTRY( "th_HomeBtn_d_btry_gry" );
	LOADANIMBTRY( "th_HomeBtn_d_btry_red" );// make sure to set the battery to white before red.  if you do grey -> red it looks bad
	LOADANIMBTRY( "th_HomeBtn_d_btry_wht" );

	// just make all the batteries white for now
	btryObj[0]->SetAnimation( "th_HomeBtn_d_btry_wht" );
	btryObj[1]->SetAnimation( "th_HomeBtn_d_btry_wht" );
	btryObj[2]->SetAnimation( "th_HomeBtn_d_btry_wht" );
	btryObj[3]->SetAnimation( "th_HomeBtn_d_btry_wht" );

	btryObj[0]->Start();
	btryObj[1]->Start();
	btryObj[2]->Start();
	btryObj[3]->Start();



	//btnBottomBar->BindMaterials( lyt->Materials() );
	//LOADANIM( btnTopBar, "th_HomeBtn_d_hmMenu_strt" );

	//dlgObj->BindPane( lyt->FindPane( "N_Dialog" ) );

	//LOADANIM( testObj, "th_HomeBtn_d_hmMenu_bar_in" );
	//LOADANIM( testObj, "th_HomeBtn_d_optn_bar_in" );		// looks like the big wiimote and the settings it has are starting to come up from the bottom of the screen
	//LOADANIM( testObj, "th_HomeBtn_d_hmMenu_strt" );		// everything fades in and slides onto the screen asd stuff like that

	LOADANIM( btnTopBar, "th_HomeBtn_d_hmMenu_strt" );		// everything fades in and slides onto the screen asd stuff like that
	btnBottomBar->AddAnimation( anim );
	btnWiimote->AddAnimation( anim );
	btn1->AddAnimation( anim );

	LOADANIM( btnTopBar, "th_HomeBtn_d_hmMenu_fnsh" );		// fade out and go back to what we were doing
	btnBottomBar->AddAnimation( anim );
	btnWiimote->AddAnimation( anim );
	btn1->AddAnimation( anim );


	// setup "wii menu" button
	LOADANIM( btn1, "th_HomeBtn_d_cntBtn_in" );
	btn1->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM( btn1, "th_HomeBtn_d_cntBtn_out" );
	btn1->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM( btn1, "th_HomeBtn_d_cntBtn_psh" );
	btn1->SetClickAnimation( anim, 0, -1 );
	btn1->SetTrigger( Button::Btn_A );

	btn1->BindMaterial( lyt->FindMaterial( "btnL_00_L" ) );
	btn1->BindMaterial( lyt->FindMaterial( "btnL_00_R" ) );
	btn1->BindMaterial( lyt->FindMaterial( "btnL_00_M" ) );
	btn1->BindMaterial( lyt->FindMaterial( "btnL_00_L_shdw" ) );
	btn1->BindMaterial( lyt->FindMaterial( "btnL_00_R_shdw" ) );
	btn1->BindMaterial( lyt->FindMaterial( "btnL_00_M_shdw" ) );

	// setup top bar button
	LOADANIM( btnTopBar, "th_HomeBtn_d_hmMenu_bar_in" );
	btnTopBar->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM( btnTopBar, "th_HomeBtn_d_hmMenu_bar_out" );
	btnTopBar->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM( btnTopBar, "th_HomeBtn_d_hmMenu_bar_psh" );
	btnTopBar->SetClickAnimation( anim, 0, -1 );
	btnTopBar->SetTrigger( Button::Btn_A );

	// setup bottom bar button
	LOADANIM2( btnBottomBar, btnWiimote, "th_HomeBtn_d_close_bar_in" );
	btnBottomBar->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM( btnBottomBar, "th_HomeBtn_d_close_bar_out" );
	btnBottomBar->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM2( btnBottomBar, btnWiimote, "th_HomeBtn_d_close_bar_psh" );
	btnBottomBar->SetClickAnimation( anim, 0, -1 );
	//btnBottomBar->SetTrigger( Button::Btn_A );

	// setup huge wiimote button
	LOADANIM( btnWiimote, "th_HomeBtn_d_cntrl_dwn" );	// the settings menu slides into the wiimote and then it slides down to the bottom of the screen
	LOADANIM( btnWiimote, "th_HomeBtn_d_cntrl_up" );	// the controller slides up
	LOADANIM( btnWiimote, "th_HomeBtn_d_cntrl_wndw_opn" );	// wiimote settinsg menu slides out

	// setup dialog
	dlgObj->BindPane( lyt->FindPane( "back_01" ) );		// shade that makes everything behind the dialog darker
	dlgObj->BindPane( lyt->FindPane( "N_Dialog" ) );
	//dlgObj->BindMaterials( lyt->Materials() );
	LOADANIM( dlgObj, "th_HomeBtn_d_cmn_msg_in" );		// slides in from the top
	LOADANIM( dlgObj, "th_HomeBtn_d_cmn_msg_out" );		// slides out off the bottom
	LOADANIM( dlgObj, "th_HomeBtn_d_cmn_msg_rtrn" );	// slides out off the top

	// we arent drawing the shadow correctly anyways, so hide it
	SetPaneVisible( lyt, "W_DlgShade", false );

	// main text for the dialog
	strlcpy16( dlgTxt, "Return To Loader?", 30 );
	SetText( lyt, "T_Dialog", dlgTxt );

	// setup dialog buttons
	btnDlg1->BindPane( lyt->FindPane( "N_BtnA" ) );
	btnDlg2->BindPane( lyt->FindPane( "N__BtnB" ) );
	LOADANIM2( btnDlg1, btnDlg2, "th_HomeBtn_d_cmn_msg_btn_in" );
	btnDlg1->SetMouseOverAnimation( anim, 0, -1 );
	btnDlg2->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM2( btnDlg1, btnDlg2, "th_HomeBtn_d_cmn_msg_btn_out" );
	btnDlg1->SetMouseOutAnimation( anim, 0, -1 );
	btnDlg2->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM2( btnDlg1, btnDlg2, "th_HomeBtn_d_cmn_msg_btn_psh" );
	btnDlg1->SetClickAnimation( anim, 0, -1 );
	btnDlg2->SetClickAnimation( anim, 0, -1 );
	btnDlg1->SetTrigger( Button::Btn_A );
	btnDlg2->SetTrigger( Button::Btn_A );

	btnDlg1->SetEnabled( false );
	btnDlg2->SetEnabled( false );


	// set initial fadein
	btnTopBar->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );
	btnBottomBar->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );
	btnWiimote->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );
	btn1->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );

	// connect to button slots
	btnTopBar->Finished.connect( this, &HomeMenu::TopBarAnimDone );
	btnTopBar->Clicked.connect( this, &HomeMenu::TopBarClicked );
	btn1->Clicked.connect( this, &HomeMenu::CenterBtnClicked );
	btn1->Finished.connect( this, &HomeMenu::CenterBtnAnimDone );
	dlgObj->Finished.connect( this, &HomeMenu::DialogAnimDone );

	btnDlg1->Finished.connect( this, &HomeMenu::DlgBtn1AnimDone );
	btnDlg1->Clicked.connect( this, &HomeMenu::DlgBtn1Clicked );
	btnDlg2->Finished.connect( this, &HomeMenu::DlgBtn2AnimDone );
	btnDlg2->Clicked.connect( this, &HomeMenu::DlgBtn2Clicked );

	btnTopBar->SetEnabled( false );
	btnBottomBar->SetEnabled( false );
	btnWiimote->SetEnabled( false );
	btn1->SetEnabled( false );

	btnTopBar->Start();
	btnBottomBar->Start();
	btnWiimote->Start();
	btn1->Start();

	loaded = true;
	state = St_FadeIn;
	choice = Ch_None;
	return true;
}

bool HomeMenu::LoadTpl( Texture *tex, const U8Archive &arc )
{
	const u8 *file = arc.GetFile( "/arc/timg/" + tex->getName() );
	if( file )
	{
		tex->Load( file );
		return true;
	}
	return false;
}

void HomeMenu::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}

	switch( state )
	{
	case St_FadeOut:
	case St_FadeIn:
	case St_BtnIdle:
		btnTopBar->Update();
		btnBottomBar->Update();
		btnWiimote->Update();
		btn1->Update();

		btnTopBar->Advance();
		btnBottomBar->Advance();
		btnWiimote->Advance();
		btn1->Advance();

		btryObj[0]->Advance();
		btryObj[1]->Advance();
		btryObj[2]->Advance();
		btryObj[3]->Advance();
		for( int i = 0; i < 4; i++ )
		{
			if( Pad( i ).pHome() )
			{
				// fake a click on the top button
				state = St_BtnIdle;
				choice = Ch_TopBar;
				TopBarAnimDone();
			}
		}
		break;
	case St_Dialog:
		dlgObj->Advance();

		btnDlg1->Update();
		btnDlg1->Advance();
		btnDlg2->Update();
		btnDlg2->Advance();
		break;
	}

	lyt->Render( modelview, ScreenProps, widescreen );
}

void HomeMenu::Reset()
{
	// set initial fadein
	btnTopBar->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );
	btnBottomBar->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );
	btnWiimote->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );
	btn1->SetAnimation( "th_HomeBtn_d_hmMenu_strt", 0, -1, -1, false );

	btnTopBar->Start();
	btnBottomBar->Start();
	btnWiimote->Start();
	btn1->Start();

	btnTopBar->SetEnabled( false );
	btnBottomBar->SetEnabled( false );
	btnWiimote->SetEnabled( false );
	btn1->SetEnabled( false );

	btnDlg1->SetEnabled( false );
	btnDlg2->SetEnabled( false );

	state = St_FadeIn;
	choice = Ch_None;
}

void HomeMenu::TopBarClicked()
{
	if( state == St_BtnIdle )
	{
		choice = Ch_TopBar;
	}
}

void HomeMenu::TopBarAnimDone()
{
	if( state == St_FadeIn )
	{
		btnTopBar->SetEnabled( true );
		btnBottomBar->SetEnabled( true );
		btnWiimote->SetEnabled( true );
		btn1->SetEnabled( true );
		state = St_BtnIdle;
	}
	else if( state == St_BtnIdle && choice == Ch_TopBar )
	{
		choice = Ch_None;
		btnTopBar->SetEnabled( false );
		btnBottomBar->SetEnabled( false );
		btnWiimote->SetEnabled( false );
		btn1->SetEnabled( false );
		btnTopBar->SetState( Button::St_Idle );

		// fade out
		btnTopBar->SetAnimation( "th_HomeBtn_d_hmMenu_fnsh", 0, -1, -1, false );
		btnBottomBar->SetAnimation( "th_HomeBtn_d_hmMenu_fnsh", 0, -1, -1, false );
		btnWiimote->SetAnimation( "th_HomeBtn_d_hmMenu_fnsh", 0, -1, -1, false );
		btn1->SetAnimation( "th_HomeBtn_d_hmMenu_fnsh", 0, -1, -1, false );

		btnTopBar->Start();
		btnBottomBar->Start();
		btnWiimote->Start();
		btn1->Start();

		state = St_FadeOut;
	}
	else if( state == St_FadeOut )
	{
		Done();
	}
}

void HomeMenu::CenterBtnClicked()
{
	if( state == St_BtnIdle )
	{
		choice = Ch_CenterBtn;
	}
}

void HomeMenu::CenterBtnAnimDone()
{
	if( choice == Ch_CenterBtn )
	{
		choice = Ch_None;
		state = St_Dialog;
		dlgState = DSt_FadeIn;

		// disable background buttons
		btnTopBar->SetEnabled( false );
		btnBottomBar->SetEnabled( false );
		btnWiimote->SetEnabled( false );
		btn1->SetEnabled( false );

		// bring in the dialog
		dlgObj->SetAnimation( "th_HomeBtn_d_cmn_msg_in", 0, -1, -1, false );
		dlgObj->Start();
	}
}

void HomeMenu::DlgBtn1Clicked()
{
	choice = Ch_DlgLeft;
}

void HomeMenu::DlgBtn1AnimDone()
{
	if( choice == Ch_DlgLeft )
	{
		choice = Ch_None;

		// get rid of the dialog
		dlgObj->SetAnimation( "th_HomeBtn_d_cmn_msg_out", 0, -1, -1, false );
		dlgObj->Start();

		// disable dialog buttons
		btnDlg1->SetEnabled( false );
		btnDlg2->SetEnabled( false );

		dlgState = DSt_FadeOutYes;
	}
}

void HomeMenu::DlgBtn2Clicked()
{
	choice = Ch_DlgRight;
}

void HomeMenu::DlgBtn2AnimDone()
{
	if( choice == Ch_DlgRight )
	{
		choice = Ch_None;

		// get rid of the dialog
		dlgObj->SetAnimation( "th_HomeBtn_d_cmn_msg_out", 0, -1, -1, false );
		dlgObj->Start();

		// disable dialog buttons
		btnDlg1->SetEnabled( false );
		btnDlg2->SetEnabled( false );

		dlgState = DSt_FadeOutNo;
	}
}

void HomeMenu::DialogAnimDone()
{
	if( dlgState == DSt_FadeIn )
	{
		// enable dialog buttons
		btnDlg1->SetEnabled( true );
		btnDlg2->SetEnabled( true );
		dlgState = DSt_Idle;
	}
	else if( dlgState == DSt_FadeOutNo )
	{
		// disable dialog buttons
		btnDlg1->SetEnabled( false );
		btnDlg2->SetEnabled( false );

		// enable main buttons
		btnTopBar->SetEnabled( true );
		btnBottomBar->SetEnabled( true );
		btnWiimote->SetEnabled( true );
		btn1->SetEnabled( true );

		state = St_BtnIdle;
	}
	else if( dlgState == DSt_FadeOutYes )
	{
		// nothing else to do here
		ExitToWiiMenu();
	}
}
