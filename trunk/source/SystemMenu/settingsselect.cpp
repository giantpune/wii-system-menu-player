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
#include "settingsselect.h"

SettingsSelect::SettingsSelect()
	: layout( NULL ),

	  btnSettings( NULL ),
	  btnDataMan( NULL ),
	  btnSaveData( NULL ),
	  btnChannel( NULL ),
	  btnWii( NULL ),
	  btnGC( NULL )
{
}

SettingsSelect::~SettingsSelect()
{
	delete btnSettings;
	delete btnDataMan;
	delete btnSaveData;
	delete btnChannel;
	delete btnWii;
	delete btnGC;

	delete layout;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

void SettingsSelect::SetupText()
{
	SetText( layout, "T_Datamanage0_00", 253 );	// "Data Management"
	SetText( layout, "T_DataManage_01", 253 );
	SetText( layout, "T_SaveData_00", 254 );	// "Save Data"
	SetText( layout, "T_SaveData_01", 254 );
	SetText( layout, "T_Setting_00", 316 );		// "Wii Settings"
	SetText( layout, "T_Channel_00", 255 );		// "Channels"
	SetText( layout, "T_Channel_01", 255 );
	SetText( layout, "T_Wii_00", 256 );			// "Wii"
	SetText( layout, "T_Wii_01", 256 );
	SetText( layout, "T_Cube_00", 257 );		// "Nintendo GameCube"
	SetText( layout, "T_Cube_01", 257 );
}

bool SettingsSelect::Load( const u8* setupSelAshData, u32 setupSelAshSize )
{
	if( loaded )
	{
		return true;
	}

	if( !setupSelAshData || !setupSelAshSize )
	{
		return false;
	}

	U8Archive arc( setupSelAshData, setupSelAshSize );

	// create layout
	if( !(layout = LoadLayout( arc, "it_ObjSetUp_a" ) ) )
	{
		return false;
	}

	// create buttons
	u32 x, y, w, h;

	ButtonCoords( Setup_Sel_Right_SM, x, y, w, h );
	btnSettings = new QuadButton( x, y, w, h );
	btnChannel = new QuadButton( x, y, w, h );
	btnGC = new QuadButton( x, y, w, h );


	ButtonCoords( Setup_Sel_Left_SM, x, y, w, h );
	btnDataMan = new QuadButton( x, y, w, h );
	btnSaveData = new QuadButton( x, y, w, h );
	btnWii = new QuadButton( x, y, w, h );

	// load animations
	Animation *anim;
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

	// anims for buttons on the first page
	btnSettings->BindGroup( layout->FindGroup( "G_Setting_00" ) );
	btnDataMan->BindGroup( layout->FindGroup( "G_DataManage_00" ) );
	LOADANIM2( btnSettings, btnDataMan, "it_ObjSetUp_a_SetUpFoucusIn" );
	btnSettings->SetMouseOverAnimation( anim, 0, -1 );
	btnDataMan->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM2( btnSettings, btnDataMan, "it_ObjSetUp_a_SetUpFoucusOut" );
	btnSettings->SetMouseOutAnimation( anim, 0, -1 );
	btnDataMan->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM2( btnSettings, btnDataMan, "it_ObjSetUp_a_SetUpFoucusFlash" );
	btnSettings->SetClickAnimation( anim, 0, -1 );
	btnDataMan->SetClickAnimation( anim, 0, -1 );
	LOADANIM2( btnSettings, btnDataMan, "it_ObjSetUp_a_SetUpIn" );
	LOADANIM2( btnSettings, btnDataMan, "it_ObjSetUp_a_SetUpOut" );
	LOADANIM2( btnSettings, btnDataMan, "it_ObjSetUp_a_SetUpBack" );
	btnSettings->SetTrigger( Button::Btn_A );
	btnDataMan->SetTrigger( Button::Btn_A );

	// anims for buttons on the data management page
	btnSaveData->BindGroup( layout->FindGroup( "G_SaveData_00" ) );
	btnChannel->BindGroup( layout->FindGroup( "G_Channel_00" ) );
	LOADANIM2( btnSaveData, btnChannel, "it_ObjSetUp_a_DataChannelFoucusFlash" );
	btnSaveData->SetClickAnimation( anim, 0, -1 );
	btnChannel->SetClickAnimation( anim, 0, -1 );
	LOADANIM2( btnSaveData, btnChannel, "it_ObjSetUp_a_DataChannelFoucusIn" );
	btnSaveData->SetMouseOverAnimation( anim, 0, -1 );
	btnChannel->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM2( btnSaveData, btnChannel, "it_ObjSetUp_a_DataChannelFoucusOut" );
	btnSaveData->SetMouseOutAnimation( anim, 0, -1 );
	btnChannel->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM2( btnSaveData, btnChannel, "it_ObjSetUp_a_DataChannelIn" );
	LOADANIM2( btnSaveData, btnChannel, "it_ObjSetUp_a_DataChannelOut" );
	LOADANIM2( btnSaveData, btnChannel, "it_ObjSetUp_a_DataChannelBack" );
	btnSaveData->SetTrigger( Button::Btn_A );
	btnChannel->SetTrigger( Button::Btn_A );

	// setup page 3 ( savedata wii/GC )
	btnWii->BindGroup( layout->FindGroup( "G_Wii_00" ) );
	btnGC->BindGroup( layout->FindGroup( "G_Cube_00" ) );
	LOADANIM2( btnWii, btnGC, "it_ObjSetUp_a_SaveDataFoucusFlash" );
	btnWii->SetClickAnimation( anim, 0, -1 );
	btnGC->SetClickAnimation( anim, 0, -1 );
	LOADANIM2( btnWii, btnGC, "it_ObjSetUp_a_SaveDataFoucusIn" );
	btnWii->SetMouseOverAnimation( anim, 0, -1 );
	btnGC->SetMouseOverAnimation( anim, 0, -1 );
	LOADANIM2( btnWii, btnGC, "it_ObjSetUp_a_SaveDataFoucusOut" );
	btnWii->SetMouseOutAnimation( anim, 0, -1 );
	btnGC->SetMouseOutAnimation( anim, 0, -1 );
	LOADANIM2( btnWii, btnGC, "it_ObjSetUp_a_SaveDataIn" );
	LOADANIM2( btnWii, btnGC, "it_ObjSetUp_a_SaveDataOut" );
	LOADANIM2( btnWii, btnGC, "it_ObjSetUp_a_SaveDataBack" );
	btnWii->SetTrigger( Button::Btn_A );
	btnGC->SetTrigger( Button::Btn_A );

	// setup page 1
	btnSettings->SetAnimation( "it_ObjSetUp_a_SetUpIn", 0, -1, -1, false );
	btnDataMan->SetAnimation( "it_ObjSetUp_a_SetUpIn", 0, -1, -1, false );
	btnSettings->Start();
	btnDataMan->Start();

	// disable buttons not on the first page
	btnSettings->SetEnabled( false );
	btnDataMan->SetEnabled( false );
	btnChannel->SetEnabled( false );
	btnGC->SetEnabled( false );
	btnSaveData->SetEnabled( false );
	btnWii->SetEnabled( false );

	// connect some signals and slots to the buttons
	btnSettings->Finished.connect( this, &SettingsSelect::SettingsBtnFinished );

	btnDataMan->Clicked.connect( this, &SettingsSelect::BtnDataManClicked );
	btnDataMan->Finished.connect( this, &SettingsSelect::BtnDataManFinished );

	btnSaveData->Clicked.connect( this, &SettingsSelect::BtnSaveDataClicked );
	btnSaveData->Finished.connect( this, &SettingsSelect::BtnSaveDataFinished );

	btnWii->Clicked.connect( this, &SettingsSelect::BtnWiiSaveClicked );
	btnWii->Finished.connect( this, &SettingsSelect::BtnWiiSaveFinished );

	btnGC->Clicked.connect( this, &SettingsSelect::BtnGCSaveClicked );
	btnGC->Finished.connect( this, &SettingsSelect::BtnGCSaveFinished );

	btnChannel->Clicked.connect( this, &SettingsSelect::BtnChannelClicked );
	btnChannel->Finished.connect( this, &SettingsSelect::BtnChannelFinished );


	// setup translated strings
	SetupText();


	state = St_StartIn;
	loaded = true;
	return true;
}

void SettingsSelect::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}

	// draw it
	layout->Render( modelview, ScreenProps, widescreen );


	// animate and respond to user input
	switch( state )
	{
	case St_Start:
		btnSettings->Update();
		btnDataMan->Update();
		break;
	case St_DataManageIdle:
		btnSaveData->Update();
		btnChannel->Update();
		break;
	case St_SaveDataIdle:
		btnWii->Update();
		btnGC->Update();
		break;
	default:
		break;
	}
	btnSettings->Advance();
	btnDataMan->Advance();

	btnSaveData->Advance();
	btnChannel->Advance();

	btnWii->Advance();
	btnGC->Advance();
}

void SettingsSelect::SettingsBtnClicked()
{
}

void SettingsSelect::SettingsBtnFinished()
{
}

void SettingsSelect::BtnDataManClicked()
{
	if( state == St_Start )
	{
		state = St_StartOut;

		// fade out right button
		btnSettings->SetEnabled( false );
		btnSettings->SetAnimation( "it_ObjSetUp_a_SetUpOut", 0, -1, -1, false );
		btnSettings->Start();
	}
}

void SettingsSelect::BtnDataManFinished()
{
	if( state == St_StartIn )// initial fade-in is finished
	{
		btnSettings->SetEnabled( true );
		btnDataMan->SetEnabled( true );
		state = St_Start;
	}
	else if( state == St_StartOut )
	{
		state = St_DataManageIn;
		// disable this (left) button
		btnDataMan->SetEnabled( false );
		btnDataMan->SetState( Button::St_Idle );

		// show both buttons for page 2
		btnSaveData->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnChannel->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnSaveData->SetEnabled( true );
		btnChannel->SetEnabled( true );
		btnSaveData->Start();
		btnChannel->Start();
	}
}

void SettingsSelect::BtnSaveDataClicked()
{
	if( state == St_DataManageIdle )
	{
		// start fading out "Channel" button on the right
		state = St_DataManageFadeOut;
		btnChannel->SetEnabled( false );
		btnChannel->SetAnimation( "it_ObjSetUp_a_DataChannelOut", 0, -1, -1, false );
		btnChannel->Start();
	}
}

void SettingsSelect::BtnSaveDataFinished()
{
	if( state == St_DataManageIn )// initial fadein to "data management" screen is done, enable both buttons
	{
		btnSaveData->SetEnabled( true );
		btnChannel->SetEnabled( true );
		state = St_DataManageIdle;
	}
	else if( state == St_DataManageFadeOut )// start showing the wii/GC save buttons
	{
		//gprintf( "change to save data\n" );

		state = St_SaveDataIn;
		// disable this (left) button
		btnSaveData->SetEnabled( false );
		btnSaveData->SetState( Button::St_Idle );

		// show both buttons for save data page
		btnWii->SetAnimation( "it_ObjSetUp_a_SaveDataIn", 0, -1, -1, false );
		btnGC->SetAnimation( "it_ObjSetUp_a_SaveDataIn", 0, -1, -1, false );
		btnWii->Start();
		btnGC->Start();
	}
}

void SettingsSelect::BackBtnClicked()
{
	//gprintf( "SettingsSelect::BackBtnClicked(): %u\n", state );
	if( state == St_Start )
	{
		ExitSettings();
	}
	else if( state == St_DataManageIdle )
	{
		state = St_StartIn;
		// remove the 2 current buttons
		btnSaveData->SetAnimation( "it_ObjSetUp_a_DataChannelOut", 0, -1, -1, false );
		btnChannel->SetAnimation( "it_ObjSetUp_a_DataChannelOut", 0, -1, -1, false );
		btnSaveData->SetEnabled( false );
		btnChannel->SetEnabled( false );
		btnSaveData->Start();
		btnChannel->Start();

		// bring back the buttons for page 1
		btnSettings->SetAnimation( "it_ObjSetUp_a_SetUpIn", 0, -1, -1, false );
		btnDataMan->SetAnimation( "it_ObjSetUp_a_SetUpIn", 0, -1, -1, false );
		btnSettings->Start();
		btnDataMan->Start();
	}
	else if( state == St_SaveDataIdle )
	{
		state = St_DataManageIn;
		// remove the 2 current buttons
		btnWii->SetAnimation( "it_ObjSetUp_a_SaveDataOut", 0, -1, -1, false );
		btnGC->SetAnimation( "it_ObjSetUp_a_SaveDataOut", 0, -1, -1, false );
		btnWii->SetEnabled( false );
		btnGC->SetEnabled( false );
		btnWii->Start();
		btnGC->Start();

		// bring back the buttons for page 2
		btnSaveData->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnChannel->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnSaveData->Start();
		btnChannel->Start();
	}
	/*else if( state == St_WiiSaveIdle || state == St_GCSaveIdle )
	{
		// bring back the buttons for save data page
		btnWii->SetAnimation( "it_ObjSetUp_a_SaveDataIn", 0, -1, -1, false );
		btnGC->SetAnimation( "it_ObjSetUp_a_SaveDataIn", 0, -1, -1, false );
		btnWii->Start();
		btnGC->Start();

		// show the "Wii" logo
		HideWii( false );

		// signal to remove save data layouts
		if( state == St_WiiSaveIdle )
		{
			AppendWiiSaveData( false );
		}
		else
		{
			AppendGCSaveData( false );
		}
		state = St_SaveDataIn;
	}
	else if( state == St_ChannelIdle )
	{
		state = St_DataManageIn;

		// bring back the buttons for page 2
		btnSaveData->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnChannel->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnSaveData->Start();
		btnChannel->Start();

		// show the "Wii" logo
		HideWii( false );

		// signal to remove the channel management screen
		AppendChannelManager( false );
	}*/

}

void SettingsSelect::WiiSaveDone()
{
	if( state == St_WiiSaveIdle )
	{
		// bring back the buttons for save data page
		btnWii->SetAnimation( "it_ObjSetUp_a_SaveDataIn", 0, -1, -1, false );
		btnGC->SetAnimation( "it_ObjSetUp_a_SaveDataIn", 0, -1, -1, false );
		btnWii->Start();
		btnGC->Start();

		// show the "Wii" logo
		HideWii( false );

		// signal to remove save data layouts
		AppendWiiSaveData( false );
		state = St_SaveDataIn;
	}
}

void SettingsSelect::ChannelEditDone()
{
	if( state == St_ChannelIdle )
	{
		// bring back the buttons for page 2
		btnSaveData->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnChannel->SetAnimation( "it_ObjSetUp_a_DataChannelIn", 0, -1, -1, false );
		btnSaveData->Start();
		btnChannel->Start();

		// show the "Wii" logo
		HideWii( false );

		// signal to remove the channel management screen
		AppendChannelManager( false );

		state = St_DataManageIn;
	}
}

void SettingsSelect::BtnWiiSaveClicked()
{
	if( state == St_SaveDataIdle )
	{
		state = St_WiiSaveIn;
		// get rid of the GC save button
		btnGC->SetAnimation( "it_ObjSetUp_a_SaveDataOut", 0, -1, -1, false );
		btnGC->SetEnabled( false );
		btnGC->Start();
	}
}

void SettingsSelect::BtnWiiSaveFinished()
{
	if( state == St_SaveDataIn )
	{
		state = St_SaveDataIdle;
		btnWii->SetEnabled( true );
		btnGC->SetEnabled( true );
	}
	else if( state == St_WiiSaveIn )
	{
		// disable the big wii save button
		btnWii->SetEnabled( false );
		btnWii->SetState( Button::St_Idle );
		state = St_WiiSaveIdle;

		// hide the "Wii" logo in the upper right
		HideWii( true );

		// signal for the wii save data layout
		AppendWiiSaveData( true );
	}
}

void SettingsSelect::BtnGCSaveClicked()
{
	// remove this to actually use the gamecume data management
	// (which isnt implemented yet)
	return;
	if( state == St_SaveDataIdle )
	{
		state = St_GCSaveIn;
		// get rid of the Wii save button
		btnWii->SetAnimation( "it_ObjSetUp_a_SaveDataOut", 0, -1, -1, false );
		btnWii->SetEnabled( false );
		btnWii->Start();
	}
}

void SettingsSelect::BtnGCSaveFinished()
{
	if( state == St_GCSaveIn )
	{
		// disable the big GC save button
		btnGC->SetEnabled( false );
		btnGC->SetState( Button::St_Idle );
		state = St_GCSaveIdle;

		// hide the "Wii" logo in the upper right
		HideWii( true );

		// signal to append the GC save data layout
		AppendGCSaveData( true );
	}
}

void SettingsSelect::BtnChannelClicked()
{
	if( state == St_DataManageIdle )
	{
		state = St_ChannelIn;
		// get rid of the "Save data" button
		btnSaveData->SetAnimation( "it_ObjSetUp_a_DataChannelOut", 0, -1, -1, false );
		btnSaveData->SetEnabled( false );
		btnSaveData->Start();
	}
}

void SettingsSelect::BtnChannelFinished()
{
	if( state == St_ChannelIn )
	{
		// disable the big "Channel" button
		btnChannel->SetEnabled( false );
		btnChannel->SetState( Button::St_Idle );
		state = St_ChannelIdle;

		// hide the "Wii" logo in the upper right
		HideWii( true );

		// signal to append the channel management screen
		AppendChannelManager( true );
	}
}

