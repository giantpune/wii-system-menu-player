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
#ifndef SETTINGSSELECT_H
#define SETTINGSSELECT_H

#include "button.h"
#include "object.h"
#include "systemmenuresource.h"

class SettingsSelect: public SystemMenuResource
{
public:
	SettingsSelect();
	~SettingsSelect();

	bool Load( const u8* setupSelAshData, u32 setupSelAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	// connect this to the "Back" button from setupBtn.ash
	void BackBtnClicked();

	void WiiSaveDone();
	void ChannelEditDone();

	// connect this to the "Wii" text in the upper right of the setupBtn.ash to have it show & hide that text depending on what page this is showing
	sigslot::signal1< bool > HideWii;

	// signals to append/remove other resources and scenes like the wii/gc save data and the channels
	sigslot::signal1< bool > AppendWiiSaveData;
	sigslot::signal1< bool > AppendGCSaveData;
	sigslot::signal1< bool > AppendChannelManager;

	// sent when the user asks to go back to the grid view
	sigslot::signal0<> ExitSettings;

protected:
	Layout *layout;

	QuadButton *btnSettings;
	QuadButton *btnDataMan;
	QuadButton *btnSaveData;
	QuadButton *btnChannel;
	QuadButton *btnWii;
	QuadButton *btnGC;

	std::map< std::string, Animation *>brlans;

	enum State
	{
		St_StartIn,				// start page:  Data management/wii settings
		St_Start,
		St_StartOut,

		St_DataManageIn,		// data management page: save data/channels
		St_DataManageIdle,
		St_DataManageFadeOut,

		St_SaveDataIn,			// save data page: wii/gamecube
		St_SaveDataIdle,
		St_SaveDataOut,

		St_WiiSaveIn,			// Wii save data page: shows all the wii saves
		St_WiiSaveIdle,
		St_WiiSaveOut,

		St_GCSaveIn,			// GC save data page: shows all the GC saves
		St_GCSaveIdle,
		St_GCSaveOut,

		St_ChannelIn,			// Channel management page
		St_ChannelIdle,
		St_ChannelOut

	};
	State state;

	void SetupText();

	// slots for when buttons are clicked
	void SettingsBtnClicked();
	void SettingsBtnFinished();

	void BtnDataManClicked();
	void BtnDataManFinished();

	void BtnSaveDataClicked();
	void BtnSaveDataFinished();

	void BtnWiiSaveClicked();
	void BtnWiiSaveFinished();

	void BtnGCSaveClicked();
	void BtnGCSaveFinished();

	void BtnChannelClicked();
	void BtnChannelFinished();
};






#endif // SETTINGSSELECT_H
