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
#include <stdlib.h>

#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>

#include "SystemFont.h"
#include "gecko.h"
#include "tools.h"
#include "U8Archive.h"
#include "utils/nandtitle.h"

// for lack of a better place, this stuff ends up here for now

typedef struct map_entry
{
	char name[8];
	u8 hash[20];
} __attribute__((packed)) map_entry_t;

u8 *SystemFont::fontArchive = NULL;
u32 SystemFont::archiveFilesize = 0;

WiiFont *SystemFont::wbf1 = NULL;
WiiFont *SystemFont::wbf2 = NULL;

bool SystemFont::Init()
{
	// get content.map
	const char contentMapPath[] ATTRIBUTE_ALIGN(32) = "/shared1/content.map";
	u8 *contentMap = NULL;
	u32 mapsize = 0;

	NandTitle::LoadFileFromNand(contentMapPath, &contentMap, &mapsize);
	if(!contentMap)
	{
		gprintf( "!contentMap\n" );
		return false;
	}

	int fileCount = mapsize / sizeof(map_entry_t);
	map_entry_t *mapEntryList = (map_entry_t *) contentMap;

	u8 wfbHash[] = { 0x4f, 0xad, 0x97, 0xfd, 0x4a, 0x28, 0x8c, 0x47,
				   0xe0, 0x58, 0x7f, 0x3b, 0xbd, 0x29, 0x23, 0x79,
				   0xf8, 0x70, 0x9e, 0xb9 };

	// search content.map for brfna archive
	for( int i = 0; i < fileCount; i++ )
	{
		if( memcmp(mapEntryList[i].hash, wfbHash, 20 ) )
			continue;

		// Name found
		char font_filename[32] ATTRIBUTE_ALIGN(32);
		snprintf( font_filename, sizeof( font_filename ), "/shared1/%.8s.app", mapEntryList[ i ].name );

		NandTitle::LoadFileFromNand( font_filename, &fontArchive, &archiveFilesize );
		if( !fontArchive )
		{
			free(contentMap);
			return false;
		}
	}

	free( contentMap );

	// not found
	if( !fontArchive )
	{
		// shared fonts not found
		return false;
	}

	U8Archive arc( fontArchive, archiveFilesize );

	wbf1 = new WiiFont;
	wbf2 = new WiiFont;
	if( !wbf1->Load( arc.GetFile( "wbf1.brfna" ) ) || !wbf2->Load( arc.GetFile( "wbf2.brfna" ) ) )
	{
		DELETE( wbf1 );
		DELETE( wbf2 );
		return false;
	}
	wbf1->SetName( "wbf1.brfna" );
	wbf2->SetName( "wbf2.brfna" );
	wbf1->isSystemFont = true;
	wbf2->isSystemFont = true;

	return true;
}

void SystemFont::DeInit()
{
	DELETE( wbf1 );
	DELETE( wbf2 );
	FREE( fontArchive );

	archiveFilesize = 0;
}

