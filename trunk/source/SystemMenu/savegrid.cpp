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

#include "bmg.h"
#include "buttoncoords.h"
#include "savegrid.h"
#include "SaveData/savelist.h"

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
#define LOADANIM2( x, y, z )								\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc, z ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ z ] = anim;									\
		x->AddAnimation( anim );							\
		y->AddAnimation( anim );							\
	}														\
	while( 0 )

SaveGrid::SaveGrid()
	: dialogB2( NULL ),
	  dialogLyt( NULL ),
	  gridLyt( NULL ),

	  dialogBtn1( NULL ),
	  dialogBtn2( NULL ),

	  //btnArrowLeft( NULL ),
	  //btnArrowRight( NULL ),

	  btnTabWii( NULL ),
	  btnTabSD( NULL ),

	  gridObj( new Object ),
	  gridEntryObj( new Object ),
	  dlgObj( new Object ),

	  currentPage( 0 ),
	  selectedItem( 0 )


{
	freeSpacebuffer[ 0 ] = 0;
	promptBlocksBuffer[ 0 ] = 0;
}

SaveGrid::~SaveGrid()
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

	//delete btnArrowLeft;
	//delete btnArrowRight;

	delete btnTabWii;
	delete btnTabSD;

	delete gridObj;
	delete gridEntryObj;
	delete dlgObj;

	delete dialogLyt;
	delete gridLyt;
	//delete gridBtnLyt;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

void SaveGrid::SetHidden( bool hide )
{
	if( hide )
	{
		state = St_Hidden;
	}
	else
	{
		state = St_FadeIn;

		// reset grid and tabs
		gridObj->SetAnimation( "it_ObjDataEdit_a_DataIn", 0, -1, -1, false );
		gridObj->Start();

		btnTabWii->SetEnabled( false );
		btnTabSD->SetEnabled( false );

		// reset detail dialog
		dlgObj->SetAnimation( "it_DataDetail_a_SeenIn", 0, -1, -1, false );
		dlgObj->Start();
	}
}

void SaveGrid::BackButtonClicked()
{
	if( state == St_Hidden )
	{
		return;
	}
	// back button clicked while no dialogs were showing
	if( state == St_Idle )
	{
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
			dlgObj->SetAnimation( "it_DataDetail_a_SeenOut", 0, -1, -1, false );
			dlgObj->Start();
		}
	}
}

void SaveGrid::SetDialog( DialogWindow *yesNoDialog )
{
	dialogB2 = yesNoDialog;
	dialogB2->SetText( NULL, Bmg::Instance()->GetMessage( 321 ), Bmg::Instance()->GetMessage( 322 ) );// "Yes", "No"
	dialogB2->ButtonClicked.connect( this, &SaveGrid::YesOrNoClicked );
}

void SaveGrid::YesOrNoClicked( u8 btn )
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
			dlgObj->SetAnimation( "it_DataDetail_a_SeenIn", 35, -1, -1, false );
			dlgObj->Start();
			dlgState = DSt_ConfirmCopyOut;
		}
	}
}

void SaveGrid::CopyFinished( u32 status )
{
	if( state != St_Dialog )
	{
		return;
	}
	if( dlgState == DSt_Copying )
	{

	}
}

void SaveGrid::UpdateFreeSpaceText()
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

void SaveGrid::SetPage( u16 page )
{
	u16 numPages = ( saveList.size() / 15 ) + 1;
	if( page >= numPages )
	{
		gprintf( "SaveGrid::SetPage( %u ):  > %u\n", page, numPages );
		return;
	}

	Pane *pane;
	Material *mat;
	int sIdx = ((int)page * 15) - 15;

	//gprintf( "binding save icons: %u\n", saveList.size() );
	for( int i = 0; i < 45; i++, sIdx++ )
	{
		if( !( mat = gridLyts[ i ].lyt->FindMaterial( "DataBanner_00" ) )
				|| !( pane = gridLyts[ i ].lyt->FindPane( "DataBanner_00" ) ) )
		{
			gprintf( "didn\'t find material/pane\n" );
			continue;
		}
		bool enable = false;
		if( sIdx >= 0 && sIdx < (int)saveList.size() )
		{
			//gprintf( "binding texture: %016llx to %i\n", saveList[ sIdx ]->GetIconTest()->tid, i );
			mat->SetForcedTexture( saveList[ sIdx ]->GetIconTexture() );
			enable = true;
		}
		else
		{
			mat->SetForcedTexture( NULL );
		}
		pane->SetVisible( enable );
		int btnIdx = i - 15;
		if( btnIdx >= 0 && btnIdx < 15 )
		{
			gridBtns[ btnIdx ].btn->SetEnabled( enable );
		}

	}
	currentPage = page;

}

void SaveGrid::SetupText()
{
	SetText( gridLyt, "T_SelectWii_00", 317 );			// "Wii"
	SetText( gridLyt, "T_SelectWii_01", 317 );			// "Wii"
	SetText( gridLyt, "T_SelectSd_00", 318 );			// "SD Card"
	SetText( gridLyt, "T_SelectSd_01", 318 );			// "SD Card"

	SetText( dialogLyt, "T_Copy_00", 258 );				// "Copy"
	SetText( dialogLyt, "T_Del_00", 260 );				// "Erase"

	UpdateFreeSpaceText();
}

void SaveGrid::SetupPromptForSelectedSave()
{
	if( selectedItem >= saveList.size() )
	{
		gprintf( "SaveGrid::SetupPromptForSelectedSave(): out of range %u %u\n", selectedItem, saveList.size() );
		return;
	}
	BannerBin *b = saveList[ selectedItem ];
	SetText( dialogLyt, "T_Title_00", b->GetName() );
	SetText( dialogLyt, "T_Title_01", b->GetSubtitle() );

	snprintf16( promptBlocksBuffer, 8, "%u", b->blocks );
	SetText( dialogLyt, "T_Block_00", promptBlocksBuffer );

	SetPaneVisible( dialogLyt, "T_Block_01", false );
	Material *mat;
	if( ( mat = dialogLyt->FindMaterial( "Banner_00" ) ) )
	{
		mat->SetForcedTexture( b->GetBannerTexture() );
	}
}

bool SaveGrid::LoadDialog( const U8Archive &arc )
{
	u32 x, y, w, h;
	Pane *pane;
	Pane *pane2;
	if( !( dialogLyt = LoadLayout( arc, "it_DataDetail_a" ) ) )
	{
		return false;
	}

	// dunno what this one is supposed to be for, but hide it
	SetPaneVisible( dialogLyt, "Banner_01", false );

	ButtonCoords( SaveDlg_1, x, y, w, h );
	dialogBtn1 = new QuadButton( x, y, w, h );

	ButtonCoords( SaveDlg_2, x, y, w, h );
	dialogBtn2 = new QuadButton( x, y, w, h );

	dialogBtn1->SetEnabled( false );
	dialogBtn2->SetEnabled( false );

	// load animations
	Animation *anim;

	LOADANIM( dlgObj, "it_DataDetail_a_SeenIn" );
	LOADANIM( dlgObj, "it_DataDetail_a_SeenOut" );
	LOADANIM( dlgObj, "it_DataDetail_a_SelectOut" );
	//LOADANIM( dlgObj, "it_DataDetail_a_SeenOutYesOk" );

	// copy button
	LOADANIM( dialogBtn1, "it_DataDetail_a_CopyFlash" );
	dialogBtn1->SetClickAnimation( anim, 0, -1 );
	LOADANIM( dialogBtn1, "it_DataDetail_a_CopyFoucusIn" );
	dialogBtn1->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM( dialogBtn1, "it_DataDetail_a_CopyFoucusOut" );
	dialogBtn1->SetMouseOutAnimation( anim, 0, -1 );
	dialogBtn1->BindGroup( dialogLyt->FindGroup( "G_Copy" ) );
	dialogBtn1->SetTrigger( Button::Btn_A );

	// delete button
	LOADANIM( dialogBtn2, "it_DataDetail_a_DelFlash" );
	dialogBtn2->SetClickAnimation( anim, 0, -1 );
	LOADANIM( dialogBtn2, "it_DataDetail_a_DelFoucusIn" );
	dialogBtn2->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM( dialogBtn2, "it_DataDetail_a_DelFoucusOut" );
	dialogBtn2->SetMouseOutAnimation( anim, 0, -1 );
	dialogBtn2->BindGroup( dialogLyt->FindGroup( "G_Del" ) );
	dialogBtn2->SetTrigger( Button::Btn_A );

	dlgObj->BindGroup( dialogLyt->FindGroup( "G_Window" ) );
	dlgObj->SetAnimation( "it_DataDetail_a_SeenIn", 0, -1, -1, false );
	dlgObj->Start();


	// dunno what this is about.  the buttons arent where they need to be
	if( ( pane = dialogLyt->FindPane( "N_Copy_00" ) ) )
	{
		pane->SetVisible( true );
		if( ( pane2 = dialogLyt->FindPane( "N_Del" ) ) )
		{
			pane2->SetPosition( -pane->GetPosX(), pane->GetPosY() );
		}
	}

	/*
											<< "it_DataDetail_a_CopyFlash"
											<< "it_DataDetail_a_CopyFoucusIn"
											<< "it_DataDetail_a_CopyFoucusOut"
											<< "it_DataDetail_a_DelFlash"
											<< "it_DataDetail_a_DelFoucusIn"
											<< "it_DataDetail_a_DelFoucusOut"
											<< "it_DataDetail_a_MoveFlash"
											<< "it_DataDetail_a_MoveFoucusIn"
											<< "it_DataDetail_a_MoveFoucusOut"
											<< "it_DataDetail_a_SeenIn"
											<< "it_DataDetail_a_SeenOut"
											<< "it_DataDetail_a_SeenOutNo"
											<< "it_DataDetail_a_SeenOutYes"
											<< "it_DataDetail_a_SeenOutYesOk"
											<< "it_DataDetail_a_SelectOut" ) ) )
*/
	dlgObj->Finished.connect( this, &SaveGrid::DialogAnimDone );
	dialogBtn1->Clicked.connect( this, &SaveGrid::PromptBtn1Clicked );
	dialogBtn2->Clicked.connect( this, &SaveGrid::PromptBtn2Clicked );




	dlgState = DSt_FadeIn;
	return true;
}

void SaveGrid::DialogAnimDone()
{
	if( dlgState == DSt_FadeIn )
	{
		dlgState = DSt_IdleCopyDel;
		// enable these 2 lines to actually use the dialog
		//dialogBtn1->SetEnabled( true );
		//dialogBtn2->SetEnabled( true );
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

void SaveGrid::PromptBtn1Clicked()
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
		dlgObj->SetAnimation( "it_DataDetail_a_SelectOut", 0, -1, -1, false );
		dlgObj->Start();
	}
}

void SaveGrid::PromptBtn2Clicked()
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
		dlgObj->SetAnimation( "it_DataDetail_a_SelectOut", 0, -1, -1, false );
		dlgObj->Start();*/
	}
}

void SaveGrid::GridBtnClicked( u32 idx )
{
	if( state == St_Idle )
	{
		u32 nIdx = ( currentPage * 15 ) + idx;
		if( nIdx >= saveList.size() )
		{
			gprintf( "SaveGrid::GridBtnClicked(): nIdx >= saveList.size()\n" );
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
		dlgObj->SetAnimation( "it_DataDetail_a_SeenIn", 0, -1, -1, false );
		dlgObj->Start();

		dialogBtn1->SetEnabled( false );
		dialogBtn2->SetEnabled( false );
	}
}

bool SaveGrid::Load( const u8* memoryAshData, u32 memoryAshSize )
{
	if( loaded )
	{
		gprintf( "SaveGrid::Load(): already loaded\n" );
		return true;
	}

	if( !memoryAshData || !memoryAshSize )
	{
		return false;
	}

	Pane *pane;
	U8Archive arc( memoryAshData, memoryAshSize );

	if( !( gridLyt = LoadLayout( arc, "it_ObjDataEdit_a" ) ) )
	{
		return false;
	}

	u32 x, y, w, h;

	ButtonCoords( SaveTab_1, x, y, w, h );
	btnTabWii = new QuadButton( x, y, w, h );

	ButtonCoords( SaveTab_2, x, y, w, h );
	btnTabSD = new QuadButton( x, y, w, h );

	// load all the layouts for the grid
	for( int i = 0, j = 0; i < 45; i++, j++ )
	{
		u32 len;
		if( !( gridLyts[ i ].lytData = arc.GetFileAllocated( "/arc/blyt/it_ObjDataEdit_b.brlyt", &len ) ) )
		{
			return false;
		}
		if( !( gridLyts[ i ].lyt->Load( gridLyts[ i ].lytData ) )
				|| !gridLyts[ i ].lyt->LoadTextures( arc ) )
		{
			return false;
		}
		gridEntryObj->BindPane( gridLyts[ i ].lyt->FindPane( "RootPane" ) );
	}

	// setup all the buttons for the grid
	for( int i = 0, j = 0, k = 15; i < 15; i++, j++, k++ )
	{
		if( !( gridBtns[ i ].brlanMOver = LoadAnimation( arc, "it_ObjDataEdit_b_SaveDataFoucusIn" ) ) )
		{
			return false;
		}
		if( !( gridBtns[ i ].brlanMOut = LoadAnimation( arc, "it_ObjDataEdit_b_SaveDataFoucusOut" ) ) )
		{
			return false;
		}
		if( !( gridBtns[ i ].brlanClick = LoadAnimation( arc, "it_ObjDataEdit_b_SaveDataFlash" ) ) )
		{
			return false;
		}
		ButtonCoords( (SystemMenuButton)( SaveGrid_0 + j ), x, y, w, h );
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

		gridBtns[ i ].Clicked.connect( this, &SaveGrid::GridBtnClicked );

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
		}
	}

	// load animations
	Animation *anim;

	LOADANIM( gridObj, "it_ObjDataEdit_a_ArwL1" );
	LOADANIM( gridObj, "it_ObjDataEdit_a_ArwR1" );
	LOADANIM( gridObj, "it_ObjDataEdit_a_DataIn" );					// fade the whole scene in/out
	LOADANIM( gridObj, "it_ObjDataEdit_a_DataOut" );
	LOADANIM( gridObj, "it_ObjDataEdit_a_SelectIn" );				// slide in the tabs
	LOADANIM( gridObj, "it_ObjDataEdit_a_SelectSdFlash" );
	LOADANIM( gridObj, "it_ObjDataEdit_a_SelectWiiFlash" );

	// setup object for fading in all the grid buttons
	LOADANIM( gridEntryObj, "it_ObjDataEdit_b_SaveDataIn" );
	gridEntryObj->SetAnimation( "it_ObjDataEdit_b_SaveDataIn", 0, -1, -1, false );
	gridEntryObj->Start();

	// setup the grid
	gridObj->BindGroup( gridLyt->FindGroup( "G_DataAll" ) );
	gridObj->BindGroup( gridLyt->FindGroup( "G_Select" ) );
	gridObj->BindMaterials( gridLyt->Materials() );

	gridObj->SetAnimation( "it_ObjDataEdit_a_DataIn", 0, -1, -1, false );
	gridObj->Start();


	// setup the tab buttons
	btnTabWii->BindGroup( gridLyt->FindGroup( "G_SelectWii" ) );
	btnTabSD->BindPane( gridLyt->FindPane( "G_SelectSd" ) );
	btnTabWii->SetEnabled( false );
	btnTabSD->SetEnabled( false );

	btnTabWii->SetTrigger( Button::Btn_A );
	btnTabSD->SetTrigger( Button::Btn_A );

	gridObj->Finished.connect( this, &SaveGrid::GridAnimDone );
	btnTabWii->Clicked.connect( this, &SaveGrid::WiiBtnClicked );
	btnTabSD->Clicked.connect( this, &SaveGrid::SDBtnClicked );


	if( !LoadDialog( arc ) )
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

void SaveGrid::MaybeShiftToNextPage()
{
	if( state == St_Idle )
	{
		u16 numPages = ( saveList.size() / 15 ) + 1;
		if( currentPage < numPages - 1 )
		{
			state = St_ShitfLeft;

			// disable the buttons while we shift
			for( int i = 0; i < 15; i++ )
			{
				gridBtns[ i ].btn->SetEnabled( false );
			}
			gridObj->SetAnimation( "it_ObjDataEdit_a_ArwL1", 0, -1, 0, false );
			gridObj->Start();
		}
	}
}

void SaveGrid::MaybeShiftToPrevPage()
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
			gridObj->SetAnimation( "it_ObjDataEdit_a_ArwR1", 0, -1, 0, false );
			gridObj->Start();
		}
	}
}

void SaveGrid::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
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

void SaveGrid::WiiBtnClicked()
{
	if( mode == M_SD )
	{
		mode = M_Wii;
		gridObj->SetAnimation( "it_ObjDataEdit_a_SelectSdFlash", 0, -1, -1, false );
		gridObj->Start();

		BuildSaveList( BannerBin::Nand );
		SetPage( 0 );
	}
}

void SaveGrid::SDBtnClicked()
{
	//gprintf( "wii btn clicked\n" );
	if( mode == M_Wii )
	{
		mode = M_SD;
		gridObj->SetAnimation( "it_ObjDataEdit_a_SelectWiiFlash", 0, -1, -1, false );
		gridObj->Start();

		BuildSaveList( BannerBin::SD );
		SetPage( 0 );
	}
}

void SaveGrid::GridAnimDone()
{
	if( state == St_FadeIn )
	{
		state = St_Idle;

		// show the tabs in the top right
		SetPaneVisible( gridLyt, "N_Select_00", true );

		// slide the tab buttons onto the screen
		gridObj->SetAnimation( "it_ObjDataEdit_a_SelectIn", 0, -1, -1, false );
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

