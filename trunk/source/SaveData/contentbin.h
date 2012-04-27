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
#ifndef CONTENTBIN_H
#define CONTENTBIN_H

#include <gctypes.h>
#include <gccore.h>
#include <stdio.h>

#include "Banner.h"

#define SLIM_CONTENT_BIN

// class to deal with channels moved to SD card
class ContentBin
{
public:
	ContentBin();
#ifndef SLIM_CONTENT_BIN
	// data is decrypted in place inside this buffer
	//! assume that once this function has been called, the data has been modified
	bool InstallChannel( u8* stuff, u32 len, u32 pathTID );
#endif

	// decrypts the IMET header and icon and creates a banner containing that stuff
	static u8* GetIconAsBannerData( FILE* file, u32 pathTID, u32 &outSize, u32 &installedBytes );

	// get installed size for a content.bin
	static u32 GetInstalledSize( FILE *file );

protected:

	struct Header
	{
		u64 tid;
		u32 iconSize;
		u8 md5Iheader[ 0x10 ];
		u8 md5Icon[ 0x10 ];
		u32 id4;
		u64 id6_1;
		u64 id6_2;
		char zeros_1[ 0x600 ];

	}__attribute__(( packed ));

};

#endif // CONTENTBIN_H
