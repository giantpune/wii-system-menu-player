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
#ifndef DATABIN_H
#define DATABIN_H

#include <gctypes.h>
#include <gccore.h>
#include <stdio.h>

#include "bannerbin.h"
#include "list.h"

#define SLIM_SAVE_BANNER

// class to handle the encrypted wii saves (data.bin)
class DataBin
{
public:
    DataBin();
	~DataBin();

#ifndef  SLIM_SAVE_BANNER
	// this isnt done yet
	// data is decrypted in place inside this buffer
	//! assume that once this function has been called, the data has been modified
	bool LoadFromMemory( u8* stuff, u32 len, u32 pathTID );
#endif

	// decrypts just enough of the file to get the banner and tid
	//! no signature/hash/tid checks take place
	//! returns memory allocated with memalign() or NULL on error
	static u8* GetSaveBanner( FILE* file, u32 &len, u64 &tid, u32 &installedBytes );

protected:

	// 0x20 bytes
	struct Header
	{
		u64 tid;
		u32 bnrSize;
		u8 bnrAttr;
		u8 pad0;
		u8 md5[ 0x10 ];
		u16 pad1;
	}__attribute__(( packed ));

#ifndef  SLIM_SAVE_BANNER
	struct bkEntry
	{
		u32 magic;		// 0x03adf17e
		u32 size;
		u8 perm;
		u8 attr;
		u8 type;
		char name[ 0x45 ];
		u8 iv[ 0x10 ];
	}__attribute__(( packed ));

	struct FileEntry
	{
		const u8* data;
		u32 len;
		u8 attr;
		const char* path;
		FileEntry( const u8* stuff, u32 len, u8 attr, const char* path )
			: data( stuff ),
			  len( len ),
			  attr( attr ),
			  path( path )
		{
		}
	};
	List< FileEntry *>files;
#endif
};

#endif // DATABIN_H
