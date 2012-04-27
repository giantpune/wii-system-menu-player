/*
Copyright (c) 2010 - Wii Banner Player Project
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
#include <malloc.h>
#include "SystemMenu/SystemFont.h"
#include "Banner.h"
#include "utils/nandtitle.h"
#include "utils/sc.h"
#include "utils/tools.h"

Banner::Banner( const u8 *data, u32 len )
	: arc( NULL ),
	  bannerObj( NULL ),
	  iconObj( NULL ),
	  banner_bin(NULL)
	, icon_bin(NULL)
	, sound_bin(NULL)
	, sound_bin_size(0)
	, layout_banner(NULL)
	, layout_icon(NULL)
	, imetHeader( NULL )
{
	Load(data, len);
}

Banner::~Banner()
{
	delete arc;
	delete bannerObj;
	delete iconObj;
	delete layout_banner;
	delete layout_icon;

	std::map< std::string, Animation *>:: iterator it = bannerBrlans.begin(), itE = bannerBrlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}

	it = iconBrlans.begin(), itE = iconBrlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}

	if(banner_bin)
		free(banner_bin);
	if(icon_bin)
		free(icon_bin);
	if(sound_bin)
		free(sound_bin);
}

bool Banner::Load( const u8 *data, u32 len )
{
	// find imet header
	if( data && len > sizeof( IMET ) )
	{
		if( *(u32*)( data + 0x40 ) == 0x494d4554 )
		{
			imetHeader = (u8*)( data + 0x40 );
		}
		else if( *(u32*)( data + 0x80 ) == 0x494d4554 )
		{
			imetHeader = (u8*)( data + 0x80 );
		}
		else
		{
			gprintf( "no imet header found\n" );
		}

		delete arc;
		arc = new U8Archive(data, len);

		return true;
	}

	return false;
}

Object *Banner::LoadBanner()
{
	if(!arc)
		return NULL;

	if( bannerObj )
	{
		return bannerObj;
	}
	u32 arcLen;
	if( !( banner_bin = arc->GetFileAllocated( "/meta/banner.bin", &arcLen ) ) )
	{
		return NULL;
	}

	U8Archive theArc( banner_bin, arcLen );

	// create layout
	if( !( layout_banner = LoadLayout( theArc, "banner" ) ) )
	{
		return NULL;
	}

	// create object
	bannerObj = new Object;
	bannerObj->BindPane( layout_banner->FindPane( "RootPane" ) );
	bannerObj->BindMaterials( layout_banner->Materials() );

	// get animations
	std::string brlanName = "banner_Start";
	Animation *anim = LoadAnimation( theArc, brlanName );
	if( !anim )
	{
		brlanName = "banner_In";
		anim = LoadAnimation( theArc, brlanName );
		if( !anim )
		{
			brlanName = "banner_Rso0";
			anim = LoadAnimation( theArc, brlanName );
		}
	}

	// we have a starting animation
	if( anim )
	{
		layout_banner->LoadBrlanTpls( anim, theArc );
		bannerBrlans[ brlanName ] = anim;
		bannerObj->AddAnimation( anim );
		bannerObj->SetAnimation( brlanName, 0, -1, -1, false );
		bannerObj->Start();
	}

	brlanName = "banner";
	anim = LoadAnimation( theArc, brlanName );
	if( !anim )
	{
		brlanName = "banner_Loop";
		anim = LoadAnimation( theArc, brlanName );
		if( !anim )
		{
			brlanName = "banner_Rso1";
			anim = LoadAnimation( theArc, brlanName );
		}
	}

	// we have a loop animation
	if( anim )
	{
		layout_banner->LoadBrlanTpls( anim, theArc );
		bannerBrlans[ brlanName ] = anim;
		bannerObj->AddAnimation( anim );
		bannerObj->ScheduleAnimation( brlanName );
		bannerObj->Start();
	}

	return bannerObj;
}

Object *Banner::LoadIcon()
{
	if(!arc)
		return NULL;

	if( iconObj )
	{
		return iconObj;
	}

	//gprintf( "Banner::LoadIcon()\n" );
	u32 arcLen;
	bool random = true;
	if( !( icon_bin = arc->GetFileAllocated( "/meta/icon.bin", &arcLen ) ) )
	{
		return NULL;
	}

	U8Archive theArc( icon_bin, arcLen );

	// create layout
	if( !( layout_icon = LoadLayout( theArc, "icon" ) ) )
	{
		return NULL;
	}

	// create object
	iconObj = new Object;
	iconObj->BindPane( layout_icon->FindPane( "RootPane" ) );
	iconObj->BindMaterials( layout_icon->Materials() );

	// get animations
	std::string brlanName = "icon_Start";
	Animation *anim = LoadAnimation( theArc, brlanName );
	if( !anim )
	{
		brlanName = "icon_In";
		anim = LoadAnimation( theArc, brlanName );
		if( !anim )
		{
			brlanName = "icon_Rso0";
			anim = LoadAnimation( theArc, brlanName );
		}
	}

	// we have a starting animation
	if( anim )
	{
		layout_icon->LoadBrlanTpls( anim, theArc );
		iconBrlans[ brlanName ] = anim;
		iconObj->AddAnimation( anim );
		iconObj->SetAnimation( brlanName, 0, -1, -1, false );
		iconObj->Start();
		random = false;
	}

	brlanName = "icon";
	anim = LoadAnimation( theArc, brlanName );
	if( !anim )
	{
		brlanName = "icon_Loop";
		anim = LoadAnimation( theArc, brlanName );
		if( !anim )
		{
			brlanName = "icon_Rso1";
			anim = LoadAnimation( theArc, brlanName );
		}
	}

	// we have a loop animation
	if( anim )
	{
		layout_icon->LoadBrlanTpls( anim, theArc );
		iconBrlans[ brlanName ] = anim;
		iconObj->AddAnimation( anim );
		iconObj->ScheduleAnimation( brlanName );

		if( random )
		{
			iconObj->SetFrame( ((u32)rand()) % ((u32)anim->FrameCount()) );
		}

		iconObj->Start();
	}
	return iconObj;
}

Layout *Banner::LoadLayout( const U8Archive &theArc, const std::string &lytName )
{
	// read layout data
	u8 *stuff = theArc.GetFile( "/arc/blyt/" + lytName + ".brlyt" );
	if( !stuff )
	{
		return NULL;
	}

	// load layout
	Layout *ret = new Layout;
	if( !ret->Load( stuff ) )
	{
		delete ret;
		return NULL;
	}
	ret->SetLanguage( CONF_GetLanguageString() );

	// load fonts and textures
	//ret->LoadFonts( U8Archive(SystemFont::GetFont(), SystemFont::GetFontSize()) );
	ret->LoadFonts( theArc );
	if( !ret->LoadTextures( theArc ) )
	{
		delete ret;
		return NULL;
	}

	return ret;
}

Animation *Banner::LoadAnimation( const U8Archive &theArc, const std::string &lanName )
{
	u32 eNo = theArc.FileDescriptor( "/arc/anim/" + lanName + ".brlan" );
	if( !eNo )
	{
		//gprintf( "no %s.brlan\n", lanName.c_str() );
		return NULL;
	}

	u8 *stuff = theArc.GetFileFromFd( eNo );
	if( !stuff )
	{
		gprintf( "error reading %s\n", lanName.c_str() );
		return NULL;
	}
	Animation *brlan = new Animation( lanName );
	brlan->Load( (const RLAN_Header*)stuff );
	return brlan;
}

void Banner::UnloadBanner()
{
	DELETE( bannerObj );
	DELETE( layout_banner );
	FREE( banner_bin );
	std::map< std::string, Animation *>:: iterator it = bannerBrlans.begin(), itE = bannerBrlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
	bannerBrlans.clear();
}

void Banner::UnloadIcon()
{
	DELETE( iconObj );
	DELETE( layout_icon );
	FREE( icon_bin );
	std::map< std::string, Animation *>:: iterator it = iconBrlans.begin(), itE = iconBrlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
	iconBrlans.clear();
}

void Banner::UnloadSound()
{
	FREE( sound_bin );
	sound_bin_size = 0;
}

bool Banner::LoadSound()
{
	if(!arc)
		return false;

	if(sound_bin)
		free(sound_bin);

	sound_bin = arc->GetFileAllocated("/meta/sound.bin", &sound_bin_size);
	if(!sound_bin) {
		return false;
	}
	return true;
}

const char16 *Banner::GetTitle() const
{
	if( !imetHeader )
	{
		return NULL;
	}

	IMET *imet = (IMET*)imetHeader;
	int lang = CONF_GetLanguage();

	if( lang < 0 || lang > 9 || !imet->names[ lang ].title[ 0 ] )
	{
		lang = 1;
	}
	return imet->names[ lang ].title;
}

const char16 *Banner::GetSubTitle() const
{
	if( !imetHeader )
	{
		return NULL;
	}

	IMET *imet = (IMET*)imetHeader;
	int lang = CONF_GetLanguage();
	if( lang < 0 || lang > 9 || !imet->names[ lang ].subtitle[ 0 ] )
	{
		lang = 1;
	}
	return imet->names[ lang ].subtitle;
}
