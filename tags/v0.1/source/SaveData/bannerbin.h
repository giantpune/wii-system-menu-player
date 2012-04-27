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
#ifndef BANNERBIN_H
#define BANNERBIN_H

#include <gctypes.h>
#include <gccore.h>

#include "Texture.h"
#include "utils/char16.h"

// basically this is just a texture that animates itself flipbook style as it is drawn
class AnimatedTexture : public Texture
{
public:
	AnimatedTexture()
		: currentTexture( 0 ),
		  texCnt( 0 ),
		  speeds( 0 ),
		  direction( 1 ),
		  forwardAndReverse( false ),
		  delay( 0 ),
		  delay2( 6 )
	{
	}

	void Apply(u8 &tlutName, u8 map_id, u8 wrap_s, u8 wrap_t) const;
	void LoadFromRawData( u8 *data, u16 width, u16 height, u8 fmt, u8 texIdx );

	GXTexObj texObjs[ 8 ];

	mutable s8 currentTexture;
	u8 texCnt;
	u32 speeds;
	mutable int direction;
	bool forwardAndReverse;
	mutable u8 delay;
	mutable u8 delay2;
};

// class to deal with a banner.bin
//! whatever data is passed to the constructor or SetData() should be allocated with memalign().  it will be free()'d in the destructor
class BannerBin
{
public:
	explicit BannerBin( const u8* stuff = NULL, u32 len = 0, u64 tid = 0 );
	~BannerBin();
	bool SetData( const u8* stuff, u32 len );

	const char16 *GetName(){ return header ? header->name1 : NULL; }
	const char16 *GetSubtitle(){ return header ? header->name2 : NULL; }

	u64 tid;
	u16 blocks;

	enum Source
	{
		Nand,
		SD
	};
	Source source;

	Texture *GetIconTexture(){ return static_cast< Texture *>( &iconTex ); }
	Texture *GetBannerTexture(){ return &bannerTexture; }

protected:

	struct SaveHeader
	{
		u32 magic;					// WIBN
		u32 attributes;				// bit 1 is "No copy", bit 2 is "forward and reverse animation"
		u16 speeds;					// 2 bits per frame
		u16 pad0;

		u32 pad1[ 5 ];				// padding up to 0x20

		char16 name1[ 0x20 ];		// 0x40 bytes, be16 strings
		char16 name2[ 0x20 ];

		u8 bannerData[ 0x6000 ];	// 0x6000 bytes for banner texture, starting at 0xa0
	}__attribute__(( packed ));

	SaveHeader *header;

	u8 iconCnt;
	//GXTexObj iconObjs[ 8 ];	// meh, good enough
	GXTexObj bannerObj;
	AnimatedTexture iconTex;
	Texture bannerTexture;


};

#endif // BANNERBIN_H
