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
#include "ash.h"
#include "gecko.h"
#include "SaveData/databin.h"
#include "sdcontentcrypto.h"
#include "settings.h"
#include "SystemMenu/SystemMenuResources.h"
#include "SystemFont.h"
#include "U8Archive.h"
#include "utils/crc32.h"
#include "utils/fileops.h"
#include "utils/nandtitle.h"
#include "utils/sc.h"

#define INIT_ASH( x )\
	x##AshSize( 0 ), x##Ash( NULL )

SystemMenuResources *SystemMenuResources::instance = NULL;
SystemMenuResources::SystemMenuResources():
	isInited( false ),
	mainArc( NULL ),
	INIT_ASH( chanSel ),
	INIT_ASH( board ),
	INIT_ASH( chanTtl ),
	INIT_ASH( cmnBtn ),
	INIT_ASH( health ),
	backMenuData( NULL ),
	INIT_ASH( cursor ),
	INIT_ASH( fatalDlg ),
	INIT_ASH( balloon ),
	INIT_ASH( diskBann ),
	INIT_ASH( dlgWdw ),
	INIT_ASH( setupBg ),
	INIT_ASH( setupBtn ),
	INIT_ASH( setupSel ),
	INIT_ASH( memory ),
	INIT_ASH( diskThum ),
	INIT_ASH( GCBann ),
	INIT_ASH( chanEdit ),
	INIT_ASH( homeBtn1 ),
	INIT_ASH( homeBtn1L ),
	bmgData( NULL ),
	bmgDataLen( 0 ),
	fatalErrordialog( NULL )
{
}

SystemMenuResources::~SystemMenuResources()
{
	FreeEverything();
}

bool SystemMenuResources::Init()
{
	if( isInited )
	{
		return true;
	}
	int ret;
	u8* resourceArc = NULL;
	u32 resourceLen = 0;

	// System Menu
	if( !SystemFont::Init() )
	{
		return false;
	}

	// get tmd
	tmd *p_tmd = NandTitles.GetTMD( 0x100000002ull );
	if( !p_tmd )
	{
		gprintf( "can\'t get system menu TMD\n" );
		return false;
	}
	u16 idx;
	tmd_content *contents = TMD_CONTENTS( p_tmd );

	// determine which file to use for resources
	char path[ ISFS_MAXPATH ]__attribute__((aligned( 32 )));
	if( Settings::resourcePath.size() )
	{
		// user-specified path
		mainArc = new U8FileArchive( Settings::resourcePath.c_str() );
	}
	else
	{
		// default to loading from nand

		// determine resource cid
		idx = 0xffff;

		for( u16 i = 0; i < p_tmd->num_contents; i++ )
		{
			if( contents[ i ].index == 1 )
			{
				idx = i;
				break;
			}
		}
		if( idx == 0xffff )
		{
			gprintf( "SM main resource not found\n" );
			return false;
		}

		// build file path
		sprintf( path, "/title/00000001/00000002/content/%08x.app", contents[ idx ].cid );

		// create archive to get data from that file
		mainArc = new U8NandArchive( path );
	}


	// setup bmg file
	if( !SetupBmg( mainArc ) )
	{
		FreeEverything();
		return false;
	}

	// read some files
#define READ_ASH( x )																				\
	do																								\
	{																								\
		if( !( x##Ash = mainArc->GetFileAllocated( "/layout/common/" #x ".ash", & x##AshSize ) ) )	\
		{																							\
			gprintf( "Error while loading " #x ".ash\n" );											\
			FreeEverything();																		\
			return false;																			\
		}																							\
	}																								\
	while( 0 )

	// read ash files
	//READ_ASH( balloon );
	READ_ASH( dlgWdw );	// read dialog window data and keep it laying around in memory for whenever we
						// need a dialog

	// find system menu executable
	idx = 0xffff;
	for( u16 i = 0; i < p_tmd->num_contents; i++ )
	{
		if( contents[ i ].index == p_tmd->boot_index )
		{
			idx = i;
			break;
		}
	}
	if( idx == 0xffff )
	{
		gprintf( "SM executable not found\n" );
		FreeEverything();
		return false;
	}

	sprintf( path, "/title/00000001/00000002/content/%08x.app", contents[ idx ].cid );
	path[ 33 ] = '1';
	if( ( ret = NandTitles.LoadFileFromNand( path, &resourceArc, &resourceLen ) ) < 0 || !resourceArc )
	{
		path[ 33 ] = '0';
		if( ( ret = NandTitles.LoadFileFromNand( path, &resourceArc, &resourceLen ) ) < 0 || !resourceArc )
		{
			gprintf( "Error reading executable from nand: %i\n", ret );
			FreeEverything();
			return false;
		}
	}

	// find and decompress the fatal message handler
	u32* exeStart = (u32*)resourceArc, *exeEnd = (u32*)( ( resourceArc + resourceLen ) - 0x5a20 );
	while( exeStart < exeEnd )
	{
		if( exeStart[ 0 ] == 0x41534830 )// found an ASH file
		{
			// decompress it
			u32 ashSize = 0x5a20;
			u8* ashStuff = DecompressAsh( (const u8*)exeStart, ashSize );
			if( ashStuff )
			{
				// make sure it has the fatal error crap in it
				// it looks really stupid creating the dialog inside this loop and deleting it over and over,
				// but really, this should be the first, and only one it finds
				fatalErrordialog = new FatalErrordialog;
				if( fatalErrordialog->Load( ashStuff, ashSize ) )
				{
					fatalDlgAsh = ashStuff;
					fatalDlgAshSize = ashSize;
					break;
				}
				DELETE( fatalErrordialog );
			}
		}
		exeStart++;
	}

	if( !fatalErrordialog )
	{
		gprintf( "failed to find the fatal error handler\n" );
		FreeEverything();
		return false;
	}
	fatalErrordialog->SetMessage( NULL );

	// get some stuff for the save banners & SD channels
	exeStart = (u32*)resourceArc;
	exeEnd = (u32*)( ( resourceArc + resourceLen ) - 0x400 );
	bool foundCerts[ 4 ] = { false, false, false, false };
	while( exeStart < exeEnd )
	{
		if( exeStart[ 0 ] == 0x74780414 && Crc32( ((const u8*)exeStart) - 0x10, 0x400 ) == 0x0b65bd72 )
		{
			memcpy( &ca_dpki, ((const u8*)exeStart) - 0x10, 0x400 );
			foundCerts[ 0 ] = true;
		}
		else if( exeStart[ 0 ] == 0x16ff4f7a && Crc32( ((const u8*)exeStart) - 0x10, 0x400 ) == 0xca213498 )
		{
			memcpy( &ca_ppki, ((const u8*)exeStart) - 0x10, 0x400 );
			foundCerts[ 1 ] = true;
		}
		else if( exeStart[ 0 ] == 0xf2a869f7 && Crc32( ((const u8*)exeStart) - 0x10, 0x240 ) == 0xb15e343c )
		{
			memcpy( &ms_dpki, ((const u8*)exeStart) - 0x10, 0x240 );
			foundCerts[ 2 ] = true;
		}
		else if( exeStart[ 0 ] == 0x45381a4f && Crc32( ((const u8*)exeStart) - 0x10, 0x240 ) == 0x4715ed4c )
		{
			memcpy( &ms_ppki, ((const u8*)exeStart) - 0x10, 0x240 );
			foundCerts[ 3 ] = true;
		}
		exeStart++;
	}
	if( !foundCerts[ 0 ]
			|| !foundCerts[ 1 ]
			|| !foundCerts[ 2 ]
			|| !foundCerts[ 3 ] )
	{
		gprintf( "error getting certs for EC shit\n" );
		return false;
	}


	// done with the executable
	free( resourceArc );

	isInited = true;
	return true;
}

HomeMenu *SystemMenuResources::CreateHomeMenu()
{
	char path[ 65 ];
	snprintf( path, sizeof( path ), "/layout/%s/homeBtn1.ash", CONF_GetLanguageString() );
	if( (!homeBtn1Ash && !( homeBtn1Ash = mainArc->GetFileAllocated( "/layout/common/homeBtn1.ash", &homeBtn1AshSize ) ) )
			|| ( !homeBtn1LAsh && !( homeBtn1LAsh = mainArc->GetFileAllocated( path, &homeBtn1LAshSize ) ) ) )
	{
		gprintf( "Error while loading homeBtn1.ash\n" );
		return NULL;
	}
	HomeMenu *ret = new HomeMenu;
	if( !ret->Load( homeBtn1Ash, homeBtn1AshSize, homeBtn1LAsh, homeBtn1LAshSize ) )
	{
		gprintf( "error loading homeBtn1Ash\n" );
		delete ret;
		FREE( homeBtn1Ash );
		FREE( homeBtn1LAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroyHomeMenu()
{
	FREE( homeBtn1Ash );
	FREE( homeBtn1LAsh );
}

ChannelEdit *SystemMenuResources::CreateChannelEdit()
{
	if( !chanEditAsh && !( chanEditAsh = mainArc->GetFileAllocated( "/layout/common/chanEdit.ash", &chanEditAshSize ) ) )
	{
		gprintf( "Error while loading chanEdit.ash\n" );
		return NULL;
	}
	ChannelEdit *ret = new ChannelEdit;
	if( !ret->Load( chanEditAsh, chanEditAshSize ) )
	{
		gprintf( "error loading chanEditAsh\n" );
		delete ret;
		FREE( chanEditAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroyChannelEdit()
{
	FREE( chanEditAsh );
}

WiiCursors *SystemMenuResources::Cursors()
{
	if( !cursorAsh && !( cursorAsh = mainArc->GetFileAllocated( "/layout/common/cursor.ash", &cursorAshSize ) ) )
	{
		gprintf( "Error while loading cursor.ash\n" );
		return NULL;
	}
	WiiCursors *ret = new WiiCursors;
	if( !ret->Load( cursorAsh, cursorAshSize ) )
	{
		gprintf( "error loading cursorAsh\n" );
		delete ret;
		FREE( cursorAsh );
		return NULL;
	}
	return ret;
}

GCBanner *SystemMenuResources::CreateGCBanner()
{
	if( !GCBannAsh && !( GCBannAsh = mainArc->GetFileAllocated( "/layout/common/GCBann.ash", &GCBannAshSize ) ) )
	{
		gprintf( "Error while loading GCBann.ash\n" );
		return NULL;
	}
	GCBanner *ret = new GCBanner;
	if( !ret->Load( GCBannAsh, GCBannAshSize ) )
	{
		gprintf( "error loading GCBannAsh\n" );
		delete ret;
		FREE( GCBannAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroyGCBanner()
{
	FREE( GCBannAsh );
}

DiscChannel *SystemMenuResources::CreateDiscChannel()
{
	if( !diskBannAsh && !( diskBannAsh = mainArc->GetFileAllocated( "/layout/common/diskBann.ash", &diskBannAshSize ) ) )
	{
		gprintf( "Error while loading diskBann.ash\n" );
		return NULL;
	}
	DiscChannel *ret = new DiscChannel;
	if( !ret->Load( diskBannAsh, diskBannAshSize ) )
	{
		gprintf( "error loading diskBannAsh\n" );
		delete ret;
		FREE( diskBannAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroyDiscChannel()
{
	FREE( diskBannAsh );
}

DiscChannelIcon *SystemMenuResources::CreateDiscChannelIcon()
{
	if( !diskThumAsh && !( diskThumAsh = mainArc->GetFileAllocated( "/layout/common/diskThum.ash", &diskThumAshSize ) ) )
	{
		gprintf( "Error while loading diskThum.ash\n" );
		return NULL;
	}
	DiscChannelIcon *ret = new DiscChannelIcon;
	if( !ret->Load( diskThumAsh, diskThumAshSize ) )
	{
		gprintf( "error loading diskThumAsh\n" );
		delete ret;
		FREE( diskThumAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroyDiscChannelIcon()
{
	FREE( diskThumAsh );
}

SaveGrid *SystemMenuResources::CreateSaveGrid()
{
	if( !memoryAsh && !( memoryAsh = mainArc->GetFileAllocated( "/layout/common/memory.ash", &memoryAshSize ) ) )
	{
		gprintf( "Error while loading memory.ash\n" );
		return NULL;
	}
	SaveGrid *ret = new SaveGrid;
	if( !ret->Load( memoryAsh, memoryAshSize ) )
	{
		gprintf( "error loading memoryAsh\n" );
		delete ret;
		FREE( memoryAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroySaveGrid()
{
	FREE( memoryAsh );
}

SettingsSelect *SystemMenuResources::CreateSettingsSelect()
{
	if( !setupSelAsh && !( setupSelAsh = mainArc->GetFileAllocated( "/layout/common/setupSel.ash", &setupSelAshSize ) ) )
	{
		gprintf( "Error while loading setupSelAsh.ash\n" );
		return NULL;
	}
	SettingsSelect *ret = new SettingsSelect;
	if( !ret->Load( setupSelAsh, setupSelAshSize ) )
	{
		gprintf( "error loading settingsBtn\n" );
		delete ret;
		FREE( setupSelAsh );
		return NULL;
	}
	return ret;
}

void SystemMenuResources::DestroySettingsSelect()
{
	FREE( setupSelAsh );
}

SettingsBtn *SystemMenuResources::CreateSettingsBtn()
{
	if( !setupBtnAsh && !( setupBtnAsh = mainArc->GetFileAllocated( "/layout/common/setupBtn.ash", &setupBtnAshSize ) ) )
	{
		gprintf( "Error while loading setupBtn.ash\n" );
		return NULL;
	}
	SettingsBtn *settingsBtn = new SettingsBtn;
	if( !settingsBtn->Load( setupBtnAsh, setupBtnAshSize ) )
	{
		gprintf( "error loading settingsBtn\n" );
		delete settingsBtn;
		FREE( setupBtnAsh );
		return NULL;
	}
	return settingsBtn;
}

void SystemMenuResources::DestroySettingsBtn()
{
	FREE( setupBtnAsh );
}

SettingsBG *SystemMenuResources::CreateSettingsBG()
{
	if( !setupBgAsh && !( setupBgAsh = mainArc->GetFileAllocated( "/layout/common/setupBg.ash", &setupBgAshSize ) ) )
	{
		gprintf( "Error while loading setupBg.ash\n" );
		return NULL;
	}
	SettingsBG *settingsBG = new SettingsBG;
	if( !settingsBG->Load( setupBgAsh, setupBgAshSize ) )
	{
		gprintf( "error loading settingsBG\n" );
		delete settingsBG;
		FREE( setupBgAsh );
		return NULL;
	}
	return settingsBG;
}

void SystemMenuResources::DestroySettingsBG()
{
	FREE( setupBgAsh );
}

DialogWindow *SystemMenuResources::CreateDialog( DialogWindow::Type t )
{
	DialogWindow *dialogWindow = new DialogWindow;
	if( !dialogWindow->Load( dlgWdwAsh, dlgWdwAshSize, t ) )
	{
		gprintf( "error loading dialogWindow\n" );
		return NULL;
	}
	return dialogWindow;
}

BannerFrame *SystemMenuResources::CreateBannerFrame()
{
	if( !chanTtlAsh && !( chanTtlAsh = mainArc->GetFileAllocated( "/layout/common/chanTtl.ash", &chanTtlAshSize ) ) )
	{
		gprintf( "Error while loading chanTtl.ash\n" );
		return NULL;
	}
	BannerFrame *bannerFrame = new BannerFrame;
	if( !bannerFrame->Load( chanTtlAsh, chanTtlAshSize ) )
	{
		gprintf( "error loading bannerFrame\n" );
		delete bannerFrame;
		FREE( chanTtlAsh );
		return NULL;
	}
	return bannerFrame;
}

void SystemMenuResources::DestroyBannerFrame()
{
	FREE( chanTtlAsh );
}

GreyBackground *SystemMenuResources::GreyBG()
{
	if( !boardAsh && !( boardAsh = mainArc->GetFileAllocated( "/layout/common/board.ash", &boardAshSize ) ) )
	{
		gprintf( "Error while loading board.ash\n" );
		return NULL;
	}
	GreyBackground *greyBg = new GreyBackground;
	if( !greyBg->Load( boardAsh, boardAshSize ) )
	{
		gprintf( "error loading grey background\n" );
		delete greyBg;
		FREE( boardAsh );
		return NULL;
	}

	return greyBg;
}

void SystemMenuResources::DestroyGreyBG()
{
	FREE( boardAsh );
}

ButtonPanel *SystemMenuResources::CreateButtonPanel()
{
	if( !cmnBtnAsh && !( cmnBtnAsh = mainArc->GetFileAllocated( "/layout/common/cmnBtn.ash", &cmnBtnAshSize ) ) )
	{
		gprintf( "Error while loading cmnBtn.ash\n" );
		return NULL;
	}
	ButtonPanel *buttonPanel = new ButtonPanel;
	if( !buttonPanel->Load(  cmnBtnAsh, cmnBtnAshSize ) )
	{
		gprintf( "error loading common buttons\n" );
		delete buttonPanel;
		FREE( cmnBtnAsh );
		return NULL;
	}
	return buttonPanel;
}

void SystemMenuResources::DestroyButtonPanel()
{
	FREE( cmnBtnAsh );
}

ChannelGrid *SystemMenuResources::CreateChannelGrid()
{
	if( !chanSelAsh && !( chanSelAsh = mainArc->GetFileAllocated( "/layout/common/chanSel.ash", &chanSelAshSize ) ) )
	{
		gprintf( "Error while loading chanSel.ash\n" );
		return NULL;
	}
	ChannelGrid *channelGrid = new ChannelGrid;
	if( !channelGrid->Load( chanSelAsh, chanSelAshSize ) )
	{
		gprintf( "error loading channel grid\n" );
		delete channelGrid;
		FREE( chanSelAsh );
		return false;
	}
	return channelGrid;
}

void SystemMenuResources::DestroyChannelGrid()
{
	FREE( chanSelAsh );
}

u8* SystemMenuResources::LoadSystemMenuExecutable( u32 *outLen )
{
	// get tmd
	tmd *p_tmd = NandTitles.GetTMD( 0x100000002ull );
	if( !p_tmd )
	{
		gprintf( "can\'t get system menu TMD\n" );
		return NULL;
	}

	// determine resource cid
	u16 idx = 0xffff;
	tmd_content *contents = TMD_CONTENTS( p_tmd );
	for( u16 i = 0; i < p_tmd->num_contents; i++ )
	{
		if( contents[ i ].index == p_tmd->boot_index )
		{
			idx = i;
			break;
		}
	}
	if( idx == 0xffff )
	{
		gprintf( "SM executable not found\n" );
		return NULL;
	}

	u8* exeBuf;
	u32 exeSize;
	int ret;
	// build file path
	char path[ ISFS_MAXPATH ]__attribute__((aligned( 32 )));
	sprintf( path, "/title/00000001/00000002/content/%08x.app", contents[ idx ].cid );
	path[ 33 ] = '1';
	if( ( ret = NandTitles.LoadFileFromNand( path, &exeBuf, &exeSize ) ) < 0 || !exeBuf )
	{
		path[ 33 ] = '0';
		if( ( ret = NandTitles.LoadFileFromNand( path, &exeBuf, &exeSize ) ) < 0 || !exeBuf )
		{
			gprintf( "Error reading executable from nand: %i\n", ret );
			return NULL;
		}
	}

	*outLen = exeSize;
	return exeBuf;
}

HealthScreen *SystemMenuResources::CreateHealthScreen()
{
	u32 exeSize;
	u8* exeBuf = LoadSystemMenuExecutable( &exeSize );
	if( !exeBuf )
	{
		return NULL;
	}

	const u8* backMenu = FindBackMenu( exeBuf, exeSize );
	if( !backMenu )
	{
		gprintf( "didnt find the back menu\n" );
		free( exeBuf );
		return NULL;
	}
	backMenuData = (u8*)memalign( 32, 0x2720 );
	if( !backMenuData )
	{
		gprintf( "out of memory\n" );
		free( exeBuf );
		return NULL;
	}
	memcpy( backMenuData, backMenu, 0x2720 );
	free( exeBuf );

	// read health screen data
	if( !( healthAsh = mainArc->GetFileAllocated( "/layout/common/health.ash", &healthAshSize ) ) )
	{
		gprintf( "Error while loading health.ash\n" );
		FREE( backMenuData );
		return false;
	}

	// create health screen
	HealthScreen *healthScreen = new HealthScreen;
	if( !healthScreen->Load( healthAsh, healthAshSize, backMenuData, 0x2720 ) )
	{
		gprintf( "error creating health screen\n" );
		delete healthScreen;
		FREE( backMenuData );
		FREE( healthAsh );
		return NULL;
	}
	return healthScreen;
}

void SystemMenuResources::DestroyHealthScreen()
{
	FREE( healthAsh );
	FREE( backMenuData );
}

bool SystemMenuResources::SetupBmg( U8Archive *arc )
{
	const char *lang = CONF_GetLanguageString();
	char path[ 64 ];

	// try to find language file using a couple different paths
	for( int i = 0; i < 4; i++ )
	{
		switch( i )
		{
		case 0:sprintf( path, "/message/%s/ipl_common.bmg", lang ); break;
		case 1:sprintf( path, "/message/%s/ipl_common_noe.bmg", lang ); break;
		case 2:strcpy( path, "/message/eng/ipl_common.bmg" );break;
		case 3:strcpy( path, "/message/eng/ipl_common_noe.bmg" );break;
		}
		bmgData = arc->GetFileAllocated( path, &bmgDataLen );
		if( bmgData )
		{
			if( !Bmg::Instance()->SetResource( bmgData, bmgDataLen ) )
			{
				gprintf( "failed to parse bmg file\n" );
				return false;
			}
			return true;
		}
	}
	gprintf( "no bmg file found\n" );
	return false;
}

const u8* SystemMenuResources::FindBackMenu( const u8* executable, u32 len )
{
	if( !executable || !len )
	{
		return NULL;
	}
	u32 tag = 0x55aa382d;
	u32 crc32Expected = 0x4452a728;
	u32 bmSize = 0x2720;
	u32 *start = (u32*)executable, *end = (u32*)( (executable + len) - bmSize );

	// search for that bastart
	while( start < end )
	{
		if( start[ 0 ] == tag )
		{
			u32 crc = Crc32( (u8*)start, bmSize );
			if( crc == crc32Expected )
			{
				return (u8*)start;
			}
		}
		start++;
	}
	return NULL;
}

void SystemMenuResources::FreeEverything()
{
	DELETE( mainArc );

	DELETE( fatalErrordialog );

	FREE( chanSelAsh );
	FREE( boardAsh );
	FREE( chanTtlAsh );
	FREE( cmnBtnAsh );
	FREE( healthAsh );
	FREE( cursorAsh );
	FREE( bmgData );
	FREE( backMenuData );
	FREE( fatalDlgAsh );
	FREE( balloonAsh );
	FREE( diskBannAsh );
	FREE( dlgWdwAsh );
	FREE( setupBgAsh );
	FREE( setupBtnAsh );
	FREE( setupSelAsh );
	FREE( memoryAsh );
	FREE( diskThumAsh );
	FREE( GCBannAsh );
	FREE( chanEditAsh );
	FREE( homeBtn1Ash );
	FREE( homeBtn1LAsh );

	SystemFont::DeInit();

}
