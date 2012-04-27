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

#include "bannerlist.h"
#include "bmg.h"
#include "buttoncoords.h"
#include "channeledit.h"
#include "sc.h"

#define LOADANIM( x, y )									\
	do														\
	{														\
		const std::string &yRef( y );						\
		if( !(anim = LoadAnimation( arc, yRef ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ yRef ] = anim;								\
		x->AddAnimation( anim );							\
	}														\
	while( 0 )

ChannelEdit::ChannelEdit()
	: prefix( "it" ),
	  dialogB2( NULL ),
	  dialogLyt( NULL ),
	  gridLyt( NULL ),

	  dialogBtn1( NULL ),
	  dialogBtn2( NULL ),
	  dialogBtnMove( NULL ),

	  btnTabWii( NULL ),
	  btnTabSD( NULL ),

	  gridObj( new Object ),
	  gridEntryObj( new Object ),
	  dlgObj( new Object ),
	  dlgWaitingObj( new Object ),

	  currentPage( 0 ),
	  selectedItem( 0 )


{
	freeSpacebuffer[ 0 ] = 0;
	promptBlocksBuffer[ 0 ] = 0;
}

ChannelEdit::~ChannelEdit()
{
	for( int i = 0; i < 15; i++ )
	{
		delete gridBtns[ i ].btn;
		delete gridBtns[ i ].brlanMOver;
		delete gridBtns[ i ].brlanMOut;
		delete gridBtns[ i ].brlanClick;
	}

	delete dialogBtn1;
	delete dialogBtn2;
	delete dialogBtnMove;

	delete btnTabWii;
	delete btnTabSD;

	delete gridObj;
	delete dlgWaitingObj;
	delete gridEntryObj;
	delete dlgObj;

	delete dialogLyt;
	delete gridLyt;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

void ChannelEdit::SetHidden( bool hide )
{
	if( hide )
	{
		state = St_Hidden;
	}
	else
	{
		state = St_FadeIn;

		// reset grid and tabs
		gridObj->SetAnimation( "it_ObjChannelEdit_a_DataIn", 0, -1, -1, false );
		gridObj->Start();

		btnTabWii->SetEnabled( false );
		btnTabSD->SetEnabled( false );

		// reset detail dialog
		dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SeenIn", 0, -1, -1, false );
		dlgObj->Start();
	}
}

void ChannelEdit::BackButtonClicked()
{
	//gprintf( "ChannelEdit::BackButtonClicked(): %i\n", state );
	if( state == St_Hidden )
	{
		return;
	}
	// back button clicked while no dialogs were showing
	if( state == St_Idle )
	{
		mode = M_Wii;
		Done();
	}
	else if( state == St_Dialog )
	{
		// just remove the dialog window and go back to the grid
		if( dlgState == DSt_IdleCopyDel )
		{
			// remove the detail dialog
			dlgState = DSt_FadeOut;
			dialogBtn1->SetEnabled( false );
			dialogBtn2->SetEnabled( false );
			dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SeenOut", 0, -1, -1, false );
			dlgObj->Start();
		}
	}
}

void ChannelEdit::SetDialog( DialogWindow *yesNoDialog )
{
	dialogB2 = yesNoDialog;
	dialogB2->SetText( NULL, Bmg::Instance()->GetMessage( 321 ), Bmg::Instance()->GetMessage( 322 ) );// "Yes", "No"
	dialogB2->ButtonClicked.connect( this, &ChannelEdit::YesOrNoClicked );
}

void ChannelEdit::ShowWaiting( bool show )
{
	SetPaneVisible( dialogLyt, "N_Wait", show );
}

void ChannelEdit::YesOrNoClicked( u8 btn )
{
	gprintf( "yesNoClicked( %u )\n", btn );
	if( state != St_Dialog )
	{
		return;
	}
	if( dlgState == DSt_ConfirmCopyIdle )
	{
		if( !btn )// "Yes"
		{
			SetText( dialogLyt, "T_Message_00", 262 );// "Copying to the SD Card... Do not remove the SD Card or power off your Wii console."
			dlgState = DSt_Copying;
		}
		else
		{
			// yes/no dialog is gone and we dont need to show any progress while copying, enable the back button
			DisableBackButton( false );

			// change from teh message to the buttons
			dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SeenIn", 35, -1, -1, false );
			dlgObj->Start();
			dlgState = DSt_ConfirmCopyOut;
		}
	}
}

void ChannelEdit::CopyFinished( u32 status )
{
	if( state != St_Dialog )
	{
		return;
	}
	if( dlgState == DSt_Copying )
	{

	}
}

void ChannelEdit::UpdateFreeSpaceText()
{
	u32 totalClusters = 0;
	u32 s1 = 0, s2 = 0;
	char path[ 65 ]__attribute__(( aligned( 32 ) ));
	for( int i = 0; i < 10; i++ )
	{
		switch( i )
		{
		case 0:strcpy( path, "/meta" );break;
		case 1:strcpy( path, "/ticket" );break;
		case 2:strcpy( path, "/title/00010000" );break;
		case 3:strcpy( path, "/title/00010001" );break;
		case 4:strcpy( path, "/title/00010003" );break;
		case 5:strcpy( path, "/title/00010004" );break;
		case 6:strcpy( path, "/title/00010005" );break;
		case 7:strcpy( path, "/title/00010006" );break;
		case 8:strcpy( path, "/title/00010007" );break;
		case 9:strcpy( path, "/shared2/title" );break;
		}

		s1 = 0, s2 = 0;
		s32 ret = ISFS_GetUsage( path, &s1, &s2 );
		if( !ret )
		{
			totalClusters += s1;
		}
	}

	u32 freeBlocks = 0;
	if( totalClusters < 0x4400 )
	{
		freeBlocks = ( 0x4400 - totalClusters ) >> 3;
	}

	strcpy16( freeSpacebuffer, Bmg::Instance()->GetMessage( 156 ) );// "Blocks Open: "
	int len = strlen16( freeSpacebuffer );
	snprintf16( &freeSpacebuffer[ len ], 0x20 - len, "%u", freeBlocks );

	SetText( gridLyt, "T_Capa_00", freeSpacebuffer );
}

void ChannelEdit::SetPage( u16 page )
{
	u16 numPages = ( bannerList.size() / 15 ) + 1;
	if( page >= numPages )
	{
		gprintf( "ChannelEdit::SetPage( %u ):  > %u\n", page, numPages );
		return;
	}

	Pane *pane;
	int sIdx = ((int)page * 15) - 15;

	//! removed unneeded banners
	for( int i = 0; i < (int)bannerList.size(); i++)
	{
		if( i < sIdx || i > sIdx + 45)
		{
			BannerAsync::RemoveBanner( bannerList[ i ]->banner );
			bannerList[ i ]->banner = NULL;
			bannerList[ i ]->IsBound = false;
		}
	}

	//gprintf( "binding save icons: %u\n", saveList.size() );
	for( int i = 0; i < 45; i++, sIdx++ )
	{
		pane = iconPanes[ i ];
		pane->UnbindAllLayouts();

		bool enable = false;
		if( sIdx >= 0 && sIdx < (int)bannerList.size() )
		{
			//! Load banner if not loaded yet
			if( !bannerList[ sIdx ]->banner )
			{
				bannerList[ sIdx ]->banner = new BannerAsync( bannerList[ sIdx ]->filepath );
			}

			//! If banner was loaded before bind it right away otherwise later when its loaded async
			else if( bannerList[ sIdx ]->banner->getIcon() )
			{
				enable = true;
				pane->BindLayout( bannerList[ sIdx ]->banner->getIcon(), false );
				bannerList[ sIdx ]->IsBound = true;
			}
			else
			{
				bannerList[ sIdx ]->IsBound = false;
			}

		}

		int btnIdx = i - 15;
		if( btnIdx >= 0 && btnIdx < 15 )
		{
			gridBtns[ btnIdx ].btn->SetEnabled( enable );
		}
	}
	currentPage = page;

}

void ChannelEdit::SetupText()
{
	SetText( gridLyt, "T_SelectWii_00", 317 );			// "Wii"
	SetText( gridLyt, "T_SelectWii_01", 317 );			// "Wii"
	SetText( gridLyt, "T_SelectSd_00", 318 );			// "SD Card"
	SetText( gridLyt, "T_SelectSd_01", 318 );			// "SD Card"

	if( prefix == "it" )// 3.2u
	{
		SetText( dialogLyt, "T_Move_00", 258 );				// "Copy"
		SetText( dialogLyt, "T_Del_00", 260 );				// "Erase"
	}
	else// 4.2u "mn"
	{
		SetText( dialogLyt, "T_Move_00", 259 );				// "Move"

		// dunno what these 2 are for
		//SetText( dialogLyt, "T_Move_off", 259 );			// "Move"
		//SetText( dialogLyt, "T_Move_off_00", 259 );			// "Move"

		SetText( dialogLyt, "T_Copy_00", 258 );				// "Copy"
		SetText( dialogLyt, "T_Del_00", 260 );				// "Erase"
	}

	UpdateFreeSpaceText();
}

void ChannelEdit::SetupPromptForSelectedSave()
{
	if( selectedItem >= bannerList.size() )
	{
		gprintf( "ChannelEdit::SetupPromptForSelectedSave(): out of range %u %u\n", selectedItem, bannerList.size() );
		return;
	}

	//! something isnt quite right with the widescreen stuff and it makes this look like ass
	//! so forcing 4:3 for now until its figured out
	bool widescreen = 0 && ( _CONF_GetAspectRatio() == CONF_ASPECT_16_9 );


	Pane *pane;
	Banner *b = bannerList[ selectedItem ]->banner;
	snprintf16( promptBlocksBuffer, 8, "%u", bannerList[ selectedItem ]->blocks );

	if( widescreen )
	{
		SetText( dialogLyt, "T_Title_02", b->GetTitle() );
		SetText( dialogLyt, "T_Title_03", b->GetSubTitle() );
		SetText( dialogLyt, "T_Block_02", promptBlocksBuffer );
		SetPaneVisible( dialogLyt, "T_Block_03", false );
	}
	else
	{
		SetText( dialogLyt, "T_Title_00", b->GetTitle() );
		SetText( dialogLyt, "T_Title_01", b->GetSubTitle() );
		SetText( dialogLyt, "T_Block_00", promptBlocksBuffer );
		SetPaneVisible( dialogLyt, "T_Block_01", false );
	}

	// something's still not right with this one.  i tried a bunch of stuff and cant get the banner
	// to be masked off and shaped like a tv
	if( ( pane = dialogLyt->FindPane( widescreen ? "N_Atari16x9" : "N_Atari4x3" ) ) )
	{
		pane->UnbindAllLayouts();
		pane->SetGxScissorsForchildLayouts( true );
		pane->BindLayout( b->getIcon(), true );
	}
}

bool ChannelEdit::LoadDialog( const U8Archive &arc, bool widescreen )
{
	u32 x, y, w, h;
	Pane *pane;
	Pane *pane2;
	if( !( dialogLyt = LoadLayout( arc, prefix + "_ChannelDetail_a" ) ) )
	{
		prefix = "mn";
		if( !( dialogLyt = LoadLayout( arc, prefix + "_ChannelDetail_a" ) ) )
		{
			return false;
		}
	}

	SetPaneVisible( dialogLyt, "N_Mask4x3", !widescreen );
	SetPaneVisible( dialogLyt, "N_Mask16x9", widescreen );

	ButtonCoords( SaveDlg_1, x, y, w, h );
	dialogBtn1 = new QuadButton( x, y, w, h );

	ButtonCoords( SaveDlg_2, x, y, w, h );
	dialogBtn2 = new QuadButton( x, y, w, h );

	dialogBtn1->SetEnabled( false );
	dialogBtn2->SetEnabled( false );

	// load animations
	Animation *anim;

	LOADANIM( dlgObj, prefix + "_ChannelDetail_a_SeenIn" );
	LOADANIM( dlgObj, prefix + "_ChannelDetail_a_SeenOut" );
	LOADANIM( dlgObj, prefix + "_ChannelDetail_a_SelectOut" );
	LOADANIM( dlgObj, prefix + "_ChannelDetail_a_CoverAlphaIn" );// this one makes the buttons fade out and the message appear
	dialogLyt->LoadBrlanTpls( anim, arc );

	LOADANIM( dlgWaitingObj, prefix + "_ChannelDetail_a_Wait" );
	dialogLyt->LoadBrlanTpls( anim, arc );
	dlgWaitingObj->BindGroup( dialogLyt->FindGroup( "G_Wait" ) );
	dlgWaitingObj->BindMaterials( dialogLyt->Materials() );
	dlgWaitingObj->SetAnimation( prefix + "_ChannelDetail_a_Wait" );
	dlgWaitingObj->Start();


	//LOADANIM( dlgObj, prefix + "_ChannelDetail_a_SeenOutYesOk" );

	if( prefix == "mn" )// some haxx so the existing code will work for the 3 button dialog in 4.2u
	{
		ButtonCoords( ChanEditDlg_Copy, x, y, w, h );
		dialogBtn1->SetCoords( x, y, w, h );

		ButtonCoords( ChanEditDlg_Delete, x, y, w, h );
		dialogBtn2->SetCoords( x, y, w, h );

		ButtonCoords( ChanEditDlg_Move, x, y, w, h );
		dialogBtnMove = new QuadButton( x, y, w, h );

		// copy button
		LOADANIM( dialogBtn1, prefix + "_ChannelDetail_a_CopyFlash" );
		dialogBtn1->SetClickAnimation( anim, 0, -1 );
		LOADANIM( dialogBtn1, prefix + "_ChannelDetail_a_CopyFoucusIn" );
		dialogBtn1->SetMouseOverAnimation( anim, 0, -1 );
		LOADANIM( dialogBtn1, prefix + "_ChannelDetail_a_CopyFoucusOut" );
		dialogBtn1->SetMouseOutAnimation( anim, 0, -1 );
		dialogBtn1->BindGroup( dialogLyt->FindGroup( "G_Copy" ) );
		dialogBtn1->SetTrigger( Button::Btn_A );

		// move button
		LOADANIM( dialogBtnMove, prefix + "_ChannelDetail_a_MoveFlash" );
		dialogBtnMove->SetClickAnimation( anim, 0, -1 );
		LOADANIM( dialogBtnMove, prefix + "_ChannelDetail_a_MoveFoucusIn" );
		dialogBtnMove->SetMouseOverAnimation( anim, 0, -1 );
		LOADANIM( dialogBtnMove, prefix + "_ChannelDetail_a_MoveFoucusOut" );
		dialogBtnMove->SetMouseOutAnimation( anim, 0, -1 );
		dialogBtnMove->BindGroup( dialogLyt->FindGroup( "G_Move" ) );
		//dialogBtnMove->BindMaterials( dialogLyt->Materials() );
		dialogBtnMove->SetTrigger( Button::Btn_A );
		dialogBtnMove->SetEnabled( false );

		// dunno what these are, but hide them
		SetPaneVisible(  dialogLyt, "T_Move_off", false );
		SetPaneVisible(  dialogLyt, "T_Move_off_00", false );
	}
	else
	{
		// copy button
		LOADANIM( dialogBtn1, prefix + "_ChannelDetail_a_MoveFlash" );
		dialogBtn1->SetClickAnimation( anim, 0, -1 );
		LOADANIM( dialogBtn1, prefix + "_ChannelDetail_a_MoveFoucusIn" );
		dialogBtn1->SetMouseOverAnimation( anim, 0, -1 );
		LOADANIM( dialogBtn1, prefix + "_ChannelDetail_a_MoveFoucusOut" );
		dialogBtn1->SetMouseOutAnimation( anim, 0, -1 );
		dialogBtn1->BindGroup( dialogLyt->FindGroup( "G_Move" ) );
		dialogBtn1->SetTrigger( Button::Btn_A );
	}




	// delete button
	LOADANIM( dialogBtn2, prefix + "_ChannelDetail_a_DelFlash" );
	dialogBtn2->SetClickAnimation( anim, 0, -1 );
	LOADANIM( dialogBtn2, prefix + "_ChannelDetail_a_DelFoucusIn" );
	dialogBtn2->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM( dialogBtn2, prefix + "_ChannelDetail_a_DelFoucusOut" );
	dialogBtn2->SetMouseOutAnimation( anim, 0, -1 );
	dialogBtn2->BindGroup( dialogLyt->FindGroup( "G_Del" ) );
	dialogBtn2->SetTrigger( Button::Btn_A );

	dlgObj->BindGroup( dialogLyt->FindGroup( "G_Window" ) );
	dlgObj->BindGroup( dialogLyt->FindGroup( "G_Cover" ) );
	dlgObj->BindGroup( dialogLyt->FindGroup( "G_Mask" ) );
	dlgObj->BindGroup( dialogLyt->FindGroup( "G_Select" ) );
	//dlgObj->BindPane( dialogLyt->FindPane( "RootPane" ) );
	//dlgObj->BindMaterials( dialogLyt->Materials() );
	//dlgObj->BindGroup( dialogLyt->FindGroup( "G_Move" ) );
	dlgObj->UnbindPane( dialogLyt->FindPane( "N_Banner4x3" ) );
	dlgObj->UnbindPane( dialogLyt->FindPane( "N_Banner16x9" ) );

	//dlgObj->Debug();



	dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SeenIn", 0, -1, -1, false );
	//dlgObj->ScheduleAnimation( prefix + "_ChannelDetail_a_CoverAlphaIn", 0, -1, -1, false );
	dlgObj->Start();


	// dunno what this is about.  the buttons arent where they need to be
	if( ( pane = dialogLyt->FindPane( "N_Move_00" ) ) )
	{
		pane->SetVisible( true );
		if( ( pane2 = dialogLyt->FindPane( "N_Del" ) ) )
		{
			pane2->SetPosition( -pane->GetPosX(), pane->GetPosY() );
		}
	}

	// hide the "waiting" pane
	ShowWaiting( false );


	dlgObj->Finished.connect( this, &ChannelEdit::DialogAnimDone );
	dialogBtn1->Clicked.connect( this, &ChannelEdit::PromptBtn1Clicked );
	dialogBtn2->Clicked.connect( this, &ChannelEdit::PromptBtn2Clicked );




	dlgState = DSt_FadeIn;
	return true;
}

void ChannelEdit::DialogAnimDone()
{
	if( dlgState == DSt_FadeIn )
	{
		dlgState = DSt_IdleCopyDel;

		// enable these 2 lines to actually use the dialog
		//dialogBtn1->SetEnabled( true );
		//dialogBtn2->SetEnabled( true );
		//gprintf( "dlg fadein done\n" );
		dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SelectOut", 0, -1, -1, false );
	}
	else if( dlgState == DSt_ConfirmCopyIn )
	{
		dlgState = DSt_ConfirmCopyIdle;
		dialogB2->SetFadeIn();

		// disable the "Back" button while the yes/no is showing
		DisableBackButton( true );
	}
	else if( dlgState == DSt_FadeOut )
	{
		// detail dialog is done sliding out, enable all the grid buttons and tabs
		for( int i = 0; i < 15; i++ )
		{
			gridBtns[ i ].btn->SetEnabled( true );
		}
		btnTabWii->SetEnabled( true );
		btnTabSD->SetEnabled( true );

		// tell the back button to start working again
		DisableBackButton( false );

		state = St_Idle;
	}
	else if( dlgState == DSt_ConfirmCopyOut )
	{
		dlgState = DSt_IdleCopyDel;
		dialogBtn1->SetEnabled( true );
		dialogBtn2->SetEnabled( true );
	}
}

void ChannelEdit::PromptBtn1Clicked()
{
	if( dlgState == DSt_IdleCopyDel )
	{
		dlgState = DSt_ConfirmCopyIn;
		// disable buttons
		dialogBtn1->SetEnabled( false );
		dialogBtn2->SetEnabled( false );

		// set text
		SetText( dialogLyt, "T_Message_00", 261 );// "Copy to the SD Card?"
		//SETTEXT( dialogLyt, "T_Message_00", 262 );// "Copying to the SD Card... Do not remove the SD Card or power off your Wii console."
		//SETTEXT( dialogLyt, "T_Message_00", 311 );// "This data can't be moved because a portion of it can't be copied."

		// show message
		dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SelectOut", 0, -1, -1, false );
		dlgObj->Start();
	}
}

void ChannelEdit::PromptBtn2Clicked()
{
	if( dlgState == DSt_IdleCopyDel )
	{
		/*dlgState = DSt_ConfirmCopyIn;
		// disable buttons
		dialogBtn1->SetEnabled( false );
		dialogBtn2->SetEnabled( false );

		// set text
		SETTEXT( dialogLyt, "T_Message_00", 261 );// "Copy to the SD Card?"
		//SETTEXT( dialogLyt, "T_Message_00", 262 );// "Copying to the SD Card... Do not remove the SD Card or power off your Wii console."
		//SETTEXT( dialogLyt, "T_Message_00", 311 );// "This data can't be moved because a portion of it can't be copied."

		// show message
		dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SelectOut", 0, -1, -1, false );
		dlgObj->Start();*/
	}
}

void ChannelEdit::GridBtnClicked( u32 idx )
{
	if( state == St_Idle )
	{
		u32 nIdx = ( currentPage * 15 ) + idx;
		if( nIdx >= bannerList.size() )
		{
			gprintf( "ChannelEdit::GridBtnClicked(): nIdx >= bannerList.size()\n" );
			return;
		}
		state = St_Dialog;
		selectedItem = nIdx;

		// disable buttons that are moving to the background
		for( int i = 0; i < 15; i++ )
		{
			gridBtns[ i ].btn->SetEnabled( false );
		}
		btnTabWii->SetEnabled( false );
		btnTabSD->SetEnabled( false );

		// setup dialog and start it animating
		SetupPromptForSelectedSave();
		dlgState = DSt_FadeIn;
		dlgObj->SetAnimation( prefix + "_ChannelDetail_a_SeenIn", 0, -1, -1, false );
		dlgObj->Start();

		dialogBtn1->SetEnabled( false );
		dialogBtn2->SetEnabled( false );
	}
}

bool ChannelEdit::Load( const u8* chanEditAshData, u32 chanEditAshSize )
{
	if( loaded )
	{
		gprintf( "ChannelEdit::Load(): already loaded\n" );
		return true;
	}

	if( !chanEditAshData || !chanEditAshSize )
	{
		return false;
	}

	Pane *pane;
	U8Archive arc( chanEditAshData, chanEditAshSize );

	if( !( gridLyt = LoadLayout( arc, "it_ObjChannelEdit_a" ) ) )
	{
		return false;
	}

	u32 x, y, w, h;

	ButtonCoords( SaveTab_1, x, y, w, h );
	btnTabWii = new QuadButton( x, y, w, h );

	ButtonCoords( SaveTab_2, x, y, w, h );
	btnTabSD = new QuadButton( x, y, w, h );

	// load all the layouts for the grid

	//! something isnt quite right with the widescreen stuff and it makes this look like ass
	//! so forcing 4:3 for now until its figured out
	bool widescreen = 0 && ( _CONF_GetAspectRatio() == CONF_ASPECT_16_9 );
	for( int i = 0, j = 0; i < 45; i++, j++ )
	{
		u32 len;
		if( !( gridLyts[ i ].lytData = arc.GetFileAllocated( "/arc/blyt/it_ObjChannelEdit_b.brlyt", &len ) ) )
		{
			return false;
		}
		if( !( gridLyts[ i ].lyt->Load( gridLyts[ i ].lytData ) )
				|| !gridLyts[ i ].lyt->LoadTextures( arc ) )
		{
			return false;
		}
		pane = gridLyts[ i ].lyt->FindPane( "RootPane" );
		if( !pane )
		{
			gprintf( "zoinks\n" );
			return false;
		}
		gridEntryObj->BindPane( pane );

		// hide/show different panes for 4:3 / 16:9
		SetPaneVisible( gridLyts[ i ].lyt, "N_Data4x3", !widescreen );
		SetPaneVisible( gridLyts[ i ].lyt, "N_Data16x9", widescreen );
		if( ( pane = gridLyts[ i ].lyt->FindPane( widescreen ? "N_Atari16x9" : "N_Atari4x3" ) ) )
		{
			pane->SetGxScissorsForchildLayouts( true );
			iconPanes << pane;
		}
	}
	if( iconPanes.size() != 45 )
	{
		gprintf( "iconPanes.size() != 45\n" );
		return false;
	}

	// setup all the buttons for the grid
	for( int i = 0, j = 0, k = 15; i < 15; i++, j++, k++ )
	{
		if( !( gridBtns[ i ].brlanMOver = LoadAnimation( arc, "it_ObjChannelEdit_b_SaveDataFoucusIn" ) ) )
		{
			return false;
		}
		if( !( gridBtns[ i ].brlanMOut = LoadAnimation( arc, "it_ObjChannelEdit_b_SaveDataFoucusOut" ) ) )
		{
			return false;
		}
		if( !( gridBtns[ i ].brlanClick = LoadAnimation( arc, "it_ObjChannelEdit_b_SaveDataFlash" ) ) )
		{
			return false;
		}
		ButtonCoords( (SystemMenuButton)( ChannelEdit_0 + j ), x, y, w, h );
		gridBtns[ i ].btn->SetCoords( x, y, w, h );
		gridBtns[ i ].btn->BindPane( gridLyts[ k ].lyt->FindPane( "RootPane" ) );
		gridBtns[ i ].btn->AddAnimation( gridBtns[ i ].brlanMOver );
		gridBtns[ i ].btn->AddAnimation( gridBtns[ i ].brlanMOut );
		gridBtns[ i ].btn->AddAnimation( gridBtns[ i ].brlanClick );

		gridBtns[ i ].btn->SetClickAnimation( gridBtns[ i ].brlanClick, 0, -1 );
		gridBtns[ i ].btn->SetMouseOverAnimation( gridBtns[ i ].brlanMOver, 0, -1 );
		gridBtns[ i ].btn->SetMouseOutAnimation( gridBtns[ i ].brlanMOut, 0, -1 );
		gridBtns[ i ].btn->SetTrigger( Button::Btn_A );
		gridBtns[ i ].btn->Start();

		gridBtns[ i ].idx = i;

		gridBtns[ i ].Clicked.connect( this, &ChannelEdit::GridBtnClicked );

	}

	// bind non-button squares to the outter pages
	int j = 0;
	for( char page = 'a'; page < 'd'; page += 2, j += 15 )
	{
		for( int i = 0; i < 15; i++, j++ )
		{
			char name[ 20 ];
			sprintf( name, "N_Data_%c_%02u", page, i );
			if( ( pane = gridLyt->FindPane( name ) ) )
			{
				pane->BindLayout( gridLyts[ j ].lyt, false );
			}

			// dunno know why, but all the channel panes seem crammed close together.  space them out a bit
			if( ( _CONF_GetAspectRatio() == CONF_ASPECT_16_9 ) )
			{
				switch( i % 5 )
				{
				case 0:pane->SetPosition( pane->GetPosX() - 60, pane->GetPosY() );break;
				case 1:pane->SetPosition( pane->GetPosX() - 30, pane->GetPosY() );break;
				case 3:pane->SetPosition( pane->GetPosX() + 30, pane->GetPosY() );break;
				case 4:pane->SetPosition( pane->GetPosX() + 60, pane->GetPosY() );break;
				}
			}
		}
	}

	// bind button layouts to the middle page
	for( int i = 0, j = 15; i < 15; i++, j++ )
	{
		char name[ 20 ];
		sprintf( name, "N_Data_b_%02u", i );
		if( ( pane = gridLyt->FindPane( name ) ) )
		{
			pane->BindLayout( gridLyts[ j ].lyt, false );

			// dunno know why, but all the channel panes seem crammed close together.  space them out a bit
			if( ( _CONF_GetAspectRatio() == CONF_ASPECT_16_9 ) )
			{
				switch( i % 5 )
				{
				case 0:pane->SetPosition( pane->GetPosX() - 60, pane->GetPosY() );break;
				case 1:pane->SetPosition( pane->GetPosX() - 30, pane->GetPosY() );break;
				case 3:pane->SetPosition( pane->GetPosX() + 30, pane->GetPosY() );break;
				case 4:pane->SetPosition( pane->GetPosX() + 60, pane->GetPosY() );break;
				}
			}
		}
	}

	// load animations
	Animation *anim;

	LOADANIM( gridObj, "it_ObjChannelEdit_a_ArwL1" );
	LOADANIM( gridObj, "it_ObjChannelEdit_a_AwrR1" );
	LOADANIM( gridObj, "it_ObjChannelEdit_a_DataIn" );					// fade the whole scene in/out
	LOADANIM( gridObj, "it_ObjChannelEdit_a_DataOut" );
	LOADANIM( gridObj, "it_ObjChannelEdit_a_SelectIn" );				// slide in the tabs
	LOADANIM( gridObj, "it_ObjChannelEdit_a_SelectSdFlash" );
	LOADANIM( gridObj, "it_ObjChannelEdit_a_SelectWiiFlash" );

	// setup object for fading in all the grid buttons
	LOADANIM( gridEntryObj, "it_ObjChannelEdit_b_SaveDataIn" );
	gridEntryObj->SetAnimation( "it_ObjChannelEdit_b_SaveDataIn", 0, -1, -1, false );
	gridEntryObj->Start();

	// setup the grid
	gridObj->BindGroup( gridLyt->FindGroup( "G_DataAll" ) );
	gridObj->BindGroup( gridLyt->FindGroup( "G_Select" ) );
	gridObj->BindMaterials( gridLyt->Materials() );

	gridObj->SetAnimation( "it_ObjChannelEdit_a_DataIn", 0, -1, -1, false );
	gridObj->Start();

	// setup the tab buttons
	btnTabWii->BindGroup( gridLyt->FindGroup( "G_SelectWii" ) );
	btnTabSD->BindPane( gridLyt->FindPane( "G_SelectSd" ) );
	btnTabWii->SetEnabled( false );
	btnTabSD->SetEnabled( false );

	btnTabWii->SetTrigger( Button::Btn_A );
	btnTabSD->SetTrigger( Button::Btn_A );

	gridObj->Finished.connect( this, &ChannelEdit::GridAnimDone );
	btnTabWii->Clicked.connect( this, &ChannelEdit::WiiBtnClicked );
	btnTabSD->Clicked.connect( this, &ChannelEdit::SDBtnClicked );


	if( !LoadDialog( arc, widescreen ) )
	{
		return false;
	}
	SetupText();
	SetPage( 0 );

	state = St_FadeIn;
	mode = M_Wii;
	loaded = true;
	selectedItem = 0;
	return true;
}

void ChannelEdit::MaybeShiftToNextPage()
{
	if( state == St_Idle )
	{
		u16 numPages = ( bannerList.size() / 15 ) + 1;
		if( currentPage < numPages - 1 )
		{
			state = St_ShitfLeft;

			// disable the buttons while we shift
			for( int i = 0; i < 15; i++ )
			{
				gridBtns[ i ].btn->SetEnabled( false );
			}
			gridObj->SetAnimation( "it_ObjChannelEdit_a_ArwL1", 0, -1, 0, false );
			gridObj->Start();
		}
	}
}

void ChannelEdit::MaybeShiftToPrevPage()
{
	if( state == St_Idle )
	{
		if( currentPage > 0 )
		{
			state = St_ShitfRight;

			// disable the buttons while we shift
			for( int i = 0; i < 15; i++ )
			{
				gridBtns[ i ].btn->SetEnabled( false );
			}
			gridObj->SetAnimation( "it_ObjChannelEdit_a_AwrR1", 0, -1, 0, false );
			gridObj->Start();
		}
	}
}

void ChannelEdit::UpdateIcons()
{
	// draw all the banners/update animations/bind any newly loaded ones
	int chIdx = ((int)currentPage * 15) - 15;
	int sIdx = -15;

	// probably dont need to draw ever banner in the list here.  only the ones on screen
	foreach( Pane *pane, iconPanes )
	{
		if( chIdx >= 0 && chIdx < (int)bannerList.size()
		   && bannerList[ chIdx ]->banner
		   && bannerList[ chIdx ]->banner->getIcon() )
		{
			if( !bannerList[ chIdx ]->IsBound )
			{
				pane->BindLayout( bannerList[ chIdx ]->banner->getIcon(), true );
				bannerList[ chIdx ]->IsBound = true;

				if( sIdx >= 0 && sIdx < 15 )
				{
					gridBtns[ sIdx ].btn->SetEnabled( true );
				}

			}

			bannerList[ chIdx ]->banner->LoadIcon()->Advance();
		}

		chIdx++;
		sIdx++;
	}
}

void ChannelEdit::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		gprintf( "!loaded\n" );
		return;
	}
	switch( state )
	{
	case St_Idle:
	{
		// read input and check for l/r/-/+
		for(int i = 0; i < 4; i++ )
		{
			if( Pad( i ).pPlus() )
			{
				MaybeShiftToNextPage();
				break;
			}
			if( Pad( i ).pMinus() )
			{
				MaybeShiftToPrevPage();
				break;
			}
		}
	}// fallthrough
	case St_FadeIn:
	case St_ShitfLeft:
	case St_ShitfRight:
	{
		gridLyt->Render( modelview, ScreenProps, widescreen );


		 // animate
		 gridObj->Advance();

		 gridEntryObj->Advance();

		 // tab buttons
		 btnTabWii->Advance();
		 btnTabWii->Update();

		 btnTabSD->Advance();
		 btnTabSD->Update();

		 // do the buttons in the grid
		 for( int i = 0; i < 15; i++ )
		 {
			 //gridBtns[ i ].lyt->Render( modelview, ScreenProps, widescreen );
			 gridBtns[ i ].btn->Advance();
			 gridBtns[ i ].btn->Update();
		 }
		 UpdateIcons();


	}
	break;
	case St_Dialog:
	{
		gridLyt->Render( modelview, ScreenProps, widescreen );
		dialogLyt->Render( modelview, ScreenProps, widescreen );
		dlgObj->Advance();

		dialogBtn1->Update();
		dialogBtn1->Advance();
		dialogBtn2->Update();
		dialogBtn2->Advance();

		if( dialogBtnMove )
		{
			dialogBtnMove->Update();
			dialogBtnMove->Advance();
		}

		dlgWaitingObj->Advance();

		UpdateIcons();

		switch( dlgState )
		{
		case DSt_ConfirmCopyIdle:
		{
			dialogB2->Render( modelview, ScreenProps, widescreen );
		}
		break;
		default:
			break;
		}
	}
	break;
	default:
		break;
	}
}

void ChannelEdit::WiiBtnClicked()
{
	if( mode == M_SD )
	{
		mode = M_Wii;
		gridObj->SetAnimation( "it_ObjChannelEdit_a_SelectSdFlash", 0, -1, -1, false );
		gridObj->Start();

		FreeBannerList();
		BuildBannerList( NandUserChannels );
		SetPage( 0 );
	}
}

void ChannelEdit::SDBtnClicked()
{
	//gprintf( "wii btn clicked\n" );
	if( mode == M_Wii )
	{
		mode = M_SD;
		gridObj->SetAnimation( "it_ObjChannelEdit_a_SelectWiiFlash", 0, -1, -1, false );
		gridObj->Start();

		FreeBannerList();
		BuildBannerList( ChannelsMovedToSD );
		SetPage( 0 );
	}
}

void ChannelEdit::GridAnimDone()
{
	if( state == St_FadeIn )
	{
		state = St_Idle;

		// show the tabs in the top right
		SetPaneVisible( gridLyt, "N_Select_00", true );

		// slide the tab buttons onto the screen
		gridObj->SetAnimation( "it_ObjChannelEdit_a_SelectIn", 0, -1, -1, false );
		gridObj->Start();

		// enable the tab buttons
		btnTabWii->SetEnabled( true );
		btnTabSD->SetEnabled( true );
	}
	else if( state == St_ShitfLeft )
	{
		state = St_Idle;
		SetPage( currentPage + 1 );
	}
	else if( state == St_ShitfRight )
	{
		state = St_Idle;
		SetPage( currentPage - 1 );
	}
}

