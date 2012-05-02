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
#include "contentbin.h"
#include "DirList.h"
#include "fileops.h"
#include "gecko.h"
#include "nandtitle.h"
#include "settings.h"

List< BannerListEntry * >bannerList;

static void AddNandBannerPaths( u32 whichOnes )
{
	for( u32 j = 0; j < 3; j++ )
	{
		if( ( ( j == 0 ) || ( j == 2 ) ) && !( whichOnes & NandUserChannels ) )
		{
			continue;
		}
		else if( ( j == 1 ) && !( whichOnes & NandSystemChannels ) )
		{
			continue;
		}
		u32 type = ( j == 0 ) ? 0x10001 : ( ( j == 1 ) ? 0x10002 : 0x10004 );
		u32 cnt = NandTitles.SetType( type );
		for( u32 i = 0; i < cnt; i++ )
		{
			u64 tid = NandTitles.Next();

			// ignore some stuff
			// photo, news, forcast hidden halfs
			if( j == 1 && ( TITLE_LOWER( tid ) == 0x48414141 || TITLE_LOWER( tid ) == 0x48414741 || TITLE_LOWER( tid ) == 0x48414641 ) )
			{
				continue;
			}

			tmd* titleTmd = NandTitles.GetTMD( tid );
			if( !titleTmd )
			{
				continue;
			}

			u16 i;
			bool ok = false;
			for( i = 0; i < titleTmd->num_contents; i++ )
			{
				if( !titleTmd->contents[ i ].index )
				{
					ok = true;
					break;
				}
			}
			if( !ok )
			{
				continue;
			}
			char pathBuf[ 65 ]__attribute__((aligned( 32 )));

			snprintf( pathBuf, sizeof( pathBuf ), "/title/%08x/%08x/content/%08x.app", TITLE_UPPER( tid ), TITLE_LOWER( tid ),
					titleTmd->contents[i].cid );

			// check for channels that have been deleted
			s32 fd = ISFS_Open( pathBuf, ISFS_OPEN_READ );
			if( fd < 0 )
			{
				continue;
			}
			ISFS_Close( fd );

			BannerListEntry *entry = new BannerListEntry( pathBuf );

			// get size
			u32 s1 = 0, s2 = 0;
			s32 ret;
			snprintf( pathBuf, sizeof( pathBuf ), "/title/%08x/%08x/content", TITLE_UPPER( tid ), TITLE_LOWER( tid ) );
			if( !(ret = ISFS_GetUsage( pathBuf, &s1, &s2 )) )
			{
				entry->blocks = RU( s1, 8 ) / 8;
			}
			else
			{
				gprintf( "ISFS_GetUsage( \"%s\" ): %i\n", pathBuf, ret );
			}
			entry->tid = tid;
			bannerList << entry;
		}
	}
}

static void AddSDBannesrToList()
{
	DirList dir( Settings::sdBannerPath.c_str(), 0, DirList::Files);

	int BannersCount = dir.GetFilecount();
	for( int i = 0; i < BannersCount; i++ )
	{
		const char *ext = strrchr( dir.GetFilepath( i ), '.');
		if( !ext || ( strcasecmp( ext, ".bnr") && strcasecmp( ext, ".app" ) ) )
		{
			gprintf( "  skipping %s\n", dir.GetFilepath( i ) );
			continue;
		}
		//if( !strcasestr( dir.GetFilepath( i ), "SGV" ) )
		//{
		//	continue;
		//}

		BannerListEntry *entry = new BannerListEntry( dir.GetFilepath( i ) );
		bannerList << entry;
	}
}

static void AddHomebrewAppsToList()
{
	DirList dir("sd:/apps/", 0, DirList::Dirs );

	int BannersCount = dir.GetFilecount();
	for( int i = 0; i < BannersCount; i++ )
	{
		char path[ 0x80 ];
		char path2[ 0x80 ];
		snprintf( path, sizeof( path ), "%s/boot.dol", dir.GetFilepath( i ) );
		FILE *f = fopen( path, "rb" );
		if( !f )
		{
			snprintf( path, sizeof( path ), "%s/boot.elf", dir.GetFilepath( i ) );
			f = fopen( path, "rb" );
			if( !f )
			{
				continue;
			}
		}
		fclose( f );

		// create xml
		snprintf( path2, sizeof( path2 ), "%s/meta.xml", dir.GetFilepath( i ) );

		BannerListEntry *entry = new BannerListEntry( dir.GetFilepath( i ) );
		entry->hbXml = new HomebrewXML( path2 );
		if( !entry->hbXml->GetName() )
		{
			entry->hbXml->SetName( path );
		}
		bannerList << entry;
	}
}

static void AddChannelsMovedToSD()
{
	DirList dir( "sd:/private/wii/title", NULL, DirList::Dirs );
	int cnt = dir.GetFilecount();
	for( int i = 0; i < cnt; i++ )
	{
		// check for content.bin
		char path[ 65 ];
		snprintf( path, sizeof( path ), "%s/content.bin", dir.GetFilepath( i ) );

		FILE *f = fopen( path, "rb" );
		if( !f )
		{
			continue;
		}

		u32 len = ContentBin::GetInstalledSize( f );
		fclose( f );

		BannerListEntry *entry = new BannerListEntry( path );
#define KiB		( 1024 )
#define MiB		( KiB * 1024 )
#define BLOCK	( MiB / 8 )

		entry->blocks = RU( len, BLOCK ) / BLOCK;

		bannerList << entry;
	}
}

void BuildBannerList( u32 whichOnes )
{
	if( Settings::useNandBanners )
	{
		AddNandBannerPaths( whichOnes );
	}

	if( ( whichOnes & DumpedSDBanners ) && Settings::useDumpedBanners )
	{
		AddSDBannesrToList();
	}
	if( ( whichOnes & HomebrewApps ) && Settings::useHomebrewForBanners )
	{
		AddHomebrewAppsToList();
	}
	if( whichOnes & ChannelsMovedToSD )
	{
		AddChannelsMovedToSD();
	}
	//AddSDBannesrToList();
}

void FreeBannerList()
{
	foreach( BannerListEntry *e, bannerList )
	{
		delete e;
	}
	bannerList.clear();
}

