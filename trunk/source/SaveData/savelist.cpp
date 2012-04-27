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
#include "databin.h"
#include "utils/DirList.h"
#include "fileops.h"
#include "malloc.h"
#include "utils/nandtitle.h"
#include "savelist.h"

List< BannerBin * >saveList;

void BuildSaveListSD();
void BuildSaveListNand();

void BuildSaveList( BannerBin::Source source )
{
	FreeSaveList();
	switch( source )
	{
	case BannerBin::Nand:
		BuildSaveListNand();
		break;
	case BannerBin::SD:
		BuildSaveListSD();
		break;
	}
}

void BuildSaveListSD()
{
	DirList dir( "sd:/private/wii/title", NULL, DirList::Dirs );
	int cnt = dir.GetFilecount();
	for( int i = 0; i < cnt; i++ )
	{
		// check for data.bin
		char path[ 65 ];
		snprintf( path, sizeof( path ), "%s/data.bin", dir.GetFilepath( i ) );
		FILE *f = fopen( path, "rb" );
		if( !f )
		{
			//gprintf( "no data.bin\n" );
			continue;
		}

		u32 len;
		u64 tid;
		u32 installedBytes;

		u8* bannerData = DataBin::GetSaveBanner( f, len, tid, installedBytes );
		fclose( f );
		if( !bannerData )
		{
			continue;
		}


		BannerBin *save = new BannerBin( NULL, 0, tid );
		if( !save->SetData( bannerData, len ) )
		{
			gprintf( "error creating save: \"%s\"\n", path );
			delete save;
			continue;
		}

#define KiB		( 1024 )
#define MiB		( KiB * 1024 )
#define BLOCK	( MiB / 8 )

		save->blocks = RU( installedBytes, BLOCK ) / BLOCK;

		save->tid = tid;
		saveList << save;
	}
}

void BuildSaveListNand()
{
	for( u32 j = 0; j < 3; j++ )
	{
		u32 type;
		switch( j )
		{
		case 0: type = 0x10000; break;
		case 1: type = 0x10001; break;
		case 2: type = 0x10004; break;
		}

		u32 cnt = NandTitles.SetType( type );
		for( u32 i = 0; i < cnt; i++ )
		{
			u64 tid = NandTitles.Next();
			char pathBuf[ 65 ]__attribute__((aligned( 32 )));

			snprintf( pathBuf, sizeof( pathBuf ), "/title/%08x/%08x/data/banner.bin", TITLE_UPPER( tid ), TITLE_LOWER( tid ) );

			u8* buf;
			u32 len;
			int ret = -1234;
			if( ( ret = NandTitle::LoadFileFromNand( pathBuf, &buf, &len ) ) < 0 || !buf )
			{
				//gprintf( "error loading: \"%s\" %i %p\n", pathBuf, ret, buf );
				continue;
			}


			//gprintf( "load %016llx\n", tid );
			BannerBin *save = new BannerBin( NULL, 0, tid );
			if( !save->SetData( buf, len ) )
			{
				gprintf( "error creating save: \"%s\"\n", pathBuf );
				delete save;
				continue;
			}

			// get size
			u32 s1 = 0, s2 = 0;
			snprintf( pathBuf, sizeof( pathBuf ), "/title/%08x/%08x/data", TITLE_UPPER( tid ), TITLE_LOWER( tid ) );
			if( !(ret = ISFS_GetUsage( pathBuf, &s1, &s2 )) )
			{
				save->blocks = RU( s1, 8 ) / 8;
			}
			else
			{
				gprintf( "ISFS_GetUsage( \"%s\" ): %i\n", pathBuf, ret );
				save->blocks = 0;
			}
			save->tid = tid;
			saveList << save;

			//return;
		}
	}
}

void FreeSaveList()
{
	foreach( BannerBin * save, saveList )
	{
		delete save;
	}
	saveList.clear();
}
