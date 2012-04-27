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
#include "bannerbin.h"
#include "gecko.h"

BannerBin::BannerBin( const u8* stuff, u32 len, u64 tid )
	: tid( tid ),
	  header( NULL ),
	  iconCnt( 0 )
{
	if( stuff && len )
	{
		SetData( stuff, len );
	}
}

BannerBin::~BannerBin()
{
	free( header );
}

bool BannerBin::SetData( const u8* stuff, u32 len )
{
	FREE( header );
	iconCnt = 0;
	if( len < 0x72a0 || ( ( len - 0x60a0 ) % 0x1200 )  )//sanity check the size.  must have enough data for the header, names, banner, and 1 icon image
	{
		gprintf( "SaveBanner::SaveBanner -> bad filesize %08x\n", len );
		return false;
	}
	header = (SaveHeader*)stuff;
	if( header->magic != 0x5749424e )//WIBN
	{
		gprintf(  "SaveBanner::SaveBanner -> bad file magic %08x\n", header->magic );
		return false;
	}

	//init the banner
	bannerTexture.LoadFromRawData( header->bannerData, 0xc0, 0x40, 5 );

	//get the images that make up the icon
	u32 start;
	for( start = 0x60a0, iconTex.texCnt = 0; iconTex.texCnt < 8 && start <= len - 0x1200; start += 0x1200 )
	{
		// check for all zeros
		bool notZero = false;
		for( u32 j = start; j < 0x1200 + start; j += 4 )
		{
			if( *(u32*)( stuff + j ) != 0 )
			{
				notZero = true;
				break;
			}
		}
		if( !notZero )
		{
			//gprintf( "its null: %u  %016llx\n", iconTex.texCnt, tid );
			break;
		}

		iconTex.LoadFromRawData( ((u8*)stuff) + start, 0x30, 0x30, 5, iconTex.texCnt );
		//GX_InitTexObj( &iconObjs[ iconCnt ], (u8*)stuff + start, 0x30, 0x30, 5, 0, 0, true );

		//gprintf( );
		if( !( ( header->speeds >> ( 2 * iconTex.texCnt++ ) ) & 3 ) )// this is the last image
		{
			break;
		}
	}
	if( !iconTex.texCnt )// shouldn't happen
	{
		gprintf( "didn\'t get any icons for this banner\n" );
	}

	iconTex.speeds = header->speeds;
	iconTex.forwardAndReverse = ( header->attributes & 0x10 ) != 0;

	iconTex.currentTexture = rand() % iconTex.texCnt;

	return true;
}

void AnimatedTexture::Apply(u8 &tlutName, u8 map_id, u8 wrap_s, u8 wrap_t) const
{
	if( !loaded )
	{
		gprintf( "AnimatedTexture::Apply(): not loaded yet\n" );
		return;
	}

	if(tlutName >= 20 || map_id >= 8)
	{
		gprintf( "AnimatedTexture::Apply(): bad parameters\n" );
		return;
	}

	// determine which texture to use
	if( !--delay2 )
	{
		delay2 = 5;
		if( delay++ > (( speeds >> ( 2 * currentTexture ) ) & 3) )
		{
			delay = 0;
			if( forwardAndReverse )
			{
				if( direction == 1 )
				{
					if( ++currentTexture >= texCnt )
					{
						currentTexture = texCnt - 1;
						direction = -1;
					}
				}
				else
				{
					if( --currentTexture < 0 )
					{
						currentTexture = 0;
						direction = 1;
					}

				}
			}
			else
			{
				if( ++currentTexture >= texCnt )
				{
					currentTexture = 0;
				}
			}
		}
	}
	//currentTexture = 0;
	//gprintf( "current texture: (%u / %u) %u %i\n", currentTexture, texCnt, forwardAndReverse, direction );

    GX_InitTexObjWrapMode( (GXTexObj *) &texObjs[ currentTexture ], wrap_s, wrap_t );
	GX_LoadTexObj( (GXTexObj *) &texObjs[ currentTexture ], map_id) ;
}


void AnimatedTexture::LoadFromRawData( u8 *data, u16 width, u16 height, u8 fmt, u8 texIdx )
{
	//gprintf( "    AnimatedTexture::LoadFromRawData( %u )\n", texIdx );
	if( !data )
	{
		return;
	}
	loaded = true;
	GX_InitTexObj( &texObjs[ texIdx ], data, width, height, fmt, 0, 0, true );
	//GX_InitTexObjLOD( &texObjs[ texIdx ], 0, 0, 0, 0,
	//						  0, 0, 0, 0 );
}
