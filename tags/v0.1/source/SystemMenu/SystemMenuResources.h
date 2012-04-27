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
#ifndef SYSTEMMENURESOURCES_H
#define SYSTEMMENURESOURCES_H

#include <gctypes.h>

#include "bmg.h"
#include "bannerframe.h"
#include "buttonpanel.h"
#include "channeledit.h"
#include "channelgrid.h"
#include "dialogwindow.h"
#include "discchannel.h"
#include "discchannelicon.h"
#include "fatalerrordialog.h"
#include "gcbanner.h"
#include "greybackground.h"
#include "healthscreen.h"
#include "homemenu.h"
#include "menuhandler.h"
#include "savegrid.h"
#include "settingsbg.h"
#include "settingsbtn.h"
#include "settingsselect.h"
#include "tooltip.h"
#include "U8Archive.h"
#include "WiiCursors.h"

class SystemMenuResources
{
public:
	static SystemMenuResources *Instance()
	{
		if( !instance )
		{
			instance = new SystemMenuResources;
		}
		return instance;
	}

	bool Init();

	// create / get / destroy resources
	HealthScreen *CreateHealthScreen();
	void DestroyHealthScreen();

	ChannelGrid *CreateChannelGrid();
	void DestroyChannelGrid();

	ButtonPanel *CreateButtonPanel();
	void DestroyButtonPanel();

	GreyBackground *GreyBG();
	void DestroyGreyBG();

	BannerFrame *CreateBannerFrame();
	void DestroyBannerFrame();

	// creates a new dialog window with "new" and returns the pointer
	// make sure to delete it when you're done with it
	DialogWindow *CreateDialog( DialogWindow::Type t );

	FatalErrordialog *FatarError(){ return fatalErrordialog; }
	WiiCursors *Cursors();

	// resources used for the settings
	SettingsBG *CreateSettingsBG();
	void DestroySettingsBG();
	SettingsBtn *CreateSettingsBtn();
	void DestroySettingsBtn();
	SettingsSelect *CreateSettingsSelect();
	void DestroySettingsSelect();
	SaveGrid *CreateSaveGrid();
	void DestroySaveGrid();

	DiscChannelIcon *CreateDiscChannelIcon();
	void DestroyDiscChannelIcon();
	DiscChannel *CreateDiscChannel();
	void DestroyDiscChannel();

	GCBanner *CreateGCBanner();
	void DestroyGCBanner();

	ChannelEdit *CreateChannelEdit();
	void DestroyChannelEdit();

	HomeMenu *CreateHomeMenu();
	void DestroyHomeMenu();


protected:
	SystemMenuResources();
	~SystemMenuResources();
	bool isInited;
	U8NandArchive *mainArc;

#define ASH_FILE( x )\
	u32 x##AshSize;\
    u8* x##Ash

	// some nested archives that contain resources and stuff
	//! chanSel.ash contains the mask for small channels, the blue highlite animation, and the static that fills the empty channels
	ASH_FILE( chanSel );

	//! board.ash contains the grey background for the system menu, among other things
	ASH_FILE( board );

	//! chanTtl.ash contains the frame and buttons that goes around the large banners
	ASH_FILE( chanTtl );

	//! cmnBtn.ash has the buttons that slide on and off the sides of the screen for the grid and messageboard
	ASH_FILE( cmnBtn );

	//! health.ash contains the health screen
	ASH_FILE( health );
	u8* backMenuData;

	//! cursor.ash has the wiimote cursor
	ASH_FILE( cursor );

	//! this one is actually compiled inside the system menu executable
	//! it is the one that has the black screen with the white text and shows the fatal errors
	ASH_FILE( fatalDlg );

	//! baloon.ash has the little tooltip bubble
	ASH_FILE( balloon );

	//! diskBann.ash contains the big disc channel
	ASH_FILE( diskBann );

	//! dlgWdw contains 5 different dialog windows
	ASH_FILE( dlgWdw );

	//! setupBg.ash contains the background used for most of the screens in the settings and data management
	ASH_FILE( setupBg );

	//! setupBtn.ash has the bars for the top and bottom of the settings along with the back button and the "Wii" logo for the upper right
	ASH_FILE( setupBtn );

	//! setupSel.ash has the big square buttons used to navigate the settings and the tabs & text for the upper left corner
	ASH_FILE( setupSel );

	//! memory.ash has 3 layouts that make up the part of the settings where you can copy/delete/move save files
	ASH_FILE( memory );

	//! diskThum.ash has 3 layouts used for the small disc channel
	ASH_FILE( diskThum );

	//! GCBann.ash contains the big gamecube banner.  theres something else there for memory cards, but we're not using it
	ASH_FILE( GCBann );

	//! chanEdit.ash contains the layouts for the move/delete channel part of the data management
	ASH_FILE( chanEdit );

	//! there are several homeBtn1.ash.  one of them holds common files, and then there is 1 for each language
	ASH_FILE( homeBtn1 );
	ASH_FILE( homeBtn1L );

	//! bmg stuff used for translating system menu strings
	u8* bmgData;
	u32 bmgDataLen;
	bool SetupBmg( U8NandArchive *arc );
	static const u8* FindBackMenu( const u8* executable, u32 len );
	static u8* LoadSystemMenuExecutable( u32 *outLen );

	// free data
	void FreeEverything();

	// i guess this is a good enough spot for this right now
	FatalErrordialog *fatalErrordialog;

	static SystemMenuResources *instance;
};

#endif // SYSTEMMENURESOURCES_H
