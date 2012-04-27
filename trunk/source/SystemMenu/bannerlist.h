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
#ifndef BANNERLIST_H
#define BANNERLIST_H

#include <string>
#include "BannerAsync.h"
#include "list.h"

using namespace std;

class BannerListEntry
{
public:
	BannerAsync *banner;
	string filepath;
	bool IsBound;

	// stuff for channels on the nand
	u64 tid;
	u32 blocks;

	// used for homebrew apps shown as banners
	HomebrewXML *hbXml;

	BannerListEntry(const string &path)
		: banner( 0 ),
		  filepath( path ),
		  IsBound( false ),
		  tid( 0ull ),
		  blocks( 0 ),
		  hbXml( NULL )
	{
	}
	virtual ~BannerListEntry()
	{
		delete hbXml;
		delete banner;
	}
};

extern List< BannerListEntry * >bannerList;

enum
{
	NandUserChannels	= 1,		// channels allowed to be moved to SD card
	NandSystemChannels	= 2,		// stupid system channels
	AllNandChannels		= NandUserChannels | NandSystemChannels,
	DumpedSDBanners		= 4,		// plain banners in sd:/banners/
	ChannelsMovedToSD	= 8,		// encrypted channels copied to SD card
	HomebrewApps		= 0x10		// show howebrew apps from Sd card as banners
};
void BuildBannerList( u32 whichOnes = AllNandChannels | DumpedSDBanners | HomebrewApps );
void FreeBannerList();

#endif // BANNERLIST_H
