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
#ifndef MENUHANDLER_H
#define MENUHANDLER_H

#include <gctypes.h>

#include "bannerlist.h"
#include "discchannel.h"
#include "dihandler.h"
#include "gui_sound.h"
#include "savelist.h"
#include "sigslot.h"
#include "SystemMenuResources.h"
#include "WiiCursors.h"

class ChannelEdit;
class HomeMenu;
class SystemMenuResources;
class SettingsSelect;
class SettingsBtn;
class SaveGrid;


// this is responsible for deciding which resources need to be drawn and when to load & destroy resources
class MenuHandler : public sigslot::has_slots<>
{
public:
	static MenuHandler *Instance()
	{
		if( !instance )
		{
			instance = new MenuHandler;
		}
		return instance;
	}

	// enters the main loop
	void Start();

private:
	enum
	{
		FatarErrorStrLen = 0x100
	};
	MenuHandler();
	~MenuHandler();
	static MenuHandler *instance;

	SystemMenuResources *resources;
	DiHandler *diHandler;

	// mutex for making sure when the DVD thread fires off callbacks,
	// we wait until after drawing stuff to make any changes to layouts and animations
	mutex_t drawMutex;

	// keep track of which state we are in
	//! i suposed this mostly wont be needed if we are using signals and slots, but you never know
	enum State
	{
		St_Exit,
		St_FatalError,
		St_HealthScreen,
		St_ChannelGrid,
		St_BigBannerFadeIn,
		St_BigBanner,
		St_BigBannerFadeOut,

		St_SettingsSelect,
		St_WiiSaves,
		St_ChannEdit
	};
	State state;
	Banner *selectedBanner;
	GuiSound *bannerSound;
	void StartBannerSound();
	void StopBannerSound();

	// banner stuff for the disc in the DVD drive
	Banner *discChannelBanner;
	u8* discChannelBannerData;
	u32 discChannelBannerDataLen;

	// which banner is selected
	int selectedIdx;

	enum DiscBannerState
	{
		DBSt_None,
		DBSt_Spinup,
		DBSt_GCBanner,
		DBSt_WiiBanner,
		//DBSt_WiiBannerWaiting,
		DBSt_Unknown
	};
	DiscBannerState discBannerState;

	//bool discIsInserted;

	// resources
	ButtonPanel *buttonPanel;
	DiscChannelIcon *dcIcon;	// little disc channel
	DiscChannel *dcBanner;		// big disc channel
	GCBanner *gcBanner;			// big GC banner
	HomeMenu *homeMenu;
	WiiCursors *cursors;
	GreyBackground *bg;
	ChannelGrid *grid;

	SettingsSelect *setupSelect;
	SettingsBtn *setupBtn;
	SaveGrid *wiiSaveGrid;
	ChannelEdit *channelEdit;

	// layout and object for whatever is being shown for the big banner
	Layout *bigBannerLayout;
	Object *bigBannerObj;

	// loop and process different menus
	void DoHealthScreen();
	void DoFatalError();
	void DoGrid();

	void DoSettingsSelect();



	// stuff for fatal errors
	void FatalError( const char* fmt, ... )  __attribute__(( format( printf, 2, 3 ) ));
	char16 fatarErrorStr[ FatarErrorStrLen ];

	// slots

	//! triggered whin the health screen/backmenu are done
	void BackmenuFinished();

	//! one of the icons in the grid was clicked
	void ChannelIconClicked( u8 col, u8 row, int index );

	//! one of the buttons in the large banner frame was clicked
	void BannerFrameLeftButtonClicked();
	void BannerFrameRightButtonClicked();

	//! request to create / destroy the save list
	void ShowWiiSaveMenu( bool show );

	//! respond to the dvd handler
	void DiscInserted( DiHandler::DiscType dt );
	void DiscEjected();
	void StartingToReadDisc();
	void GetDiscChannelBannerData( u8* data, u32 len, bool &taken );

	//! disc channel "insert disc" animation is done
	void DiscChannelAnimFinished();

	//! button to enter the settinsg was clicked
	void SettingsBtnClicked();

	//! enter/leave the wii save data management
	void EnterLeaveWiiSaveData( bool enter );

	//! enter/leave the channel management
	void EnterLeaveChannelManager( bool enter );

	//! done in the settings, show the grid again
	void LeaveSettings();

	//! draws the home menu
	void DoHomeMenu();
	bool homeMenuActive;
	void HomeMenuWantsToExit();
	void HomeMenuCancel();

	//! the generic left/right arrows are clicked
	void LeftArrowBtnClicked();
	void RightArrowBtnClicked();

    //! Zoom animation stuff
    void AnimateZoom(const Vec2f &ScreenProps, bool AnimZoomIn);

    static const int MaxAnimSteps = 30; // Animation duration in frames
    static const float fBannerWidth = 608.0f;
    static const float fBannerHeight = 448.0f;

    Mtx44 BannerProjection;
    u8 BannerAlpha;
    u8 BGAlpha;
    int AnimStep;
    bool AnimationRunning;
    float AnimPosX1, AnimPosY1;
    float AnimPosX2, AnimPosY2;
};

#endif // MENUHANDLER_H
