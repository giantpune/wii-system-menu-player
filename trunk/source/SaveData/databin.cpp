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
#include <malloc.h>
#include <string.h>

#include <ogc/machine/processor.h>

#include "databin.h"
#include "gecko.h"
#include "crc32.h"
#include "md5.h"
#include "sdcontentcrypto.h"
#include "sha1.h"

DataBin::DataBin()
{
}

DataBin::~DataBin()
{
#ifndef  SLIM_SAVE_BANNER
	foreach( FileEntry *e, files )
	{
		delete e;
	}
#endif
}

u8* DataBin::GetSaveBanner( FILE* file, u32 &len, u64 &tid, u32 &installedBytes )
{
	// remember the position
	int pos = ftell( file );


	s32 ret;
	u8* buf = (u8*)memalign( 32, 0xf0c0 );
	if( !buf )
	{
		gprintf( "enomem\n" );
		return NULL;
	}

	// seek to the beginning
	rewind( file );

	// read
	if( fread( buf, 0xf0c0, 1, file ) != 1 )
	{
		gprintf( "short read for save data\n" );
		fseek( file, pos, SEEK_SET );
		free( buf );
		return NULL;
	}

	// get file size from the bk header
	BkHeader bkHeader;
	if( fread( &bkHeader, sizeof( BkHeader ), 1, file ) != 1 )
	{
		gprintf( "short read for save data\n" );
		fseek( file, pos, SEEK_SET );
		free( buf );
		return NULL;
	}
	if( bkHeader.magic != 0x426b0001 )
	{
		gprintf( "bad bk header for save data\n" );
		fseek( file, pos, SEEK_SET );
		free( buf );
		return NULL;
	}

	// seek back to the original position
	fseek( file, pos, SEEK_SET );

	// decrypt header
	if( (ret = DecryptWithSDKey(buf, 0xf0c0, buf ) ) < 0 )
	{
		gprintf( "ES_Decrypt(): %i\n", ret );
		free( buf );
		return false;
	}

	Header *header = (Header*)buf;

	// make sure banner size makes sense
	if( header->bnrSize < 0x72a0 || header->bnrSize > 0xf0a0 || ( header->bnrSize - 0x60a0 ) % 0x1200 )
	{
		gprintf( "DataBin::GetSaveBanner(): banner size doesn\'t look right  %08x\n", header->bnrSize );
		free( buf );
		return NULL;
	}
	u8* retB = (u8*)memalign( 32, header->bnrSize );
	if( !retB )
	{
		gprintf( "enomem\n" );
		free( buf );
		return NULL;
	}
	memcpy( retB, buf + 0x20, header->bnrSize );

	// flush it because it contain texture data we're about to feed to GX
	DCFlushRange( retB, header->bnrSize );
	free( buf );

	tid = header->tid;
	len = header->bnrSize;
	installedBytes = bkHeader.size1;// not sure if we want size 1 or 2 here
	return retB;

}

#ifndef  SLIM_SAVE_BANNER
bool DataBin::LoadFromMemory( u8* stuff, u32 len, u32 pathTID )
{
	//TestShit();
	int ret;
	if( len < 0xf140 )//header + backup header
	{
		gprintf( "len is too small: %08x\n", len );
		return false;
	}

	// do bk header
	if( !VerifyBKWad( stuff + 0xf0c0, ( len - 0xf0c0 ) ) )
	{
		return false;
	}
	BkHeader *bkHeader = (BkHeader*)( stuff + 0xf0c0 );
	gprintf( "size2: %08x\n", bkHeader->size2 );

	if( len < bkHeader->size1 + 0xf140 )
	{
		gprintf( "DataBin::LoadFromMemory():bkheader size1\n" );
		return false;
	}

	// decrypt header
	if( (ret = DecryptWithSDKey( stuff, 0xf0c0, stuff ) ) < 0 )
	{
		gprintf( "DecryptWithSDKey(): %i\n", ret );
		return false;
	}
	Header *header = (Header*)stuff;

	// verify MD5
	u8 md5Actual[ 0x10 ];
	u8 md5Header[ 0x10 ];

	//! ipl::NandSDWorker::c_md5_secret_key
	u8 md5Blacker[ 0x10 ] = { 0x0E, 0x65, 0x37, 0x81, 0x99, 0xBE, 0x45, 0x17, 0xAB, 0x06, 0xEC, 0x22, 0x45, 0x1A, 0x57, 0x93 };

	memcpy( md5Header, header->md5, 0x10 );
	memcpy( header->md5, md5Blacker, 0x10 );
	MD5::GetMD5( stuff, 0xf0c0, md5Actual );
	if( memcmp( md5Actual, md5Header, 0x10 ) )
	{
		gprintf( "DataBin::LoadFromMemory(): md5 mismatch\n" );
		return false;
	}

	// verify TID
	if( ( header->tid & 0xffffffff ) != pathTID )
	{
		gprintf( "DataBin::LoadFromMemory(): TID mismatch in header\n" );
		return false;
	}

	// make sure banner size makes sense
	if( header->bnrSize < 0x72a0 || header->bnrSize > 0xf0a0 || ( header->bnrSize - 0x60a0 ) % 0x1200 )
	{
		gprintf( "DataBin::LoadFromMemory(): banner size doesn\'t look right  %08x\n", header->bnrSize );
		return false;
	}

	// add the banner to the list of files
	files << new FileEntry( stuff + 0x20, header->bnrSize, header->bnrAttr, "banner.bin" );



	//hexdump( stuff, 0x100 );
	//exit( 0 );

	// do files
	u32 cnt = bkHeader->numEntries;
	u32 pos = 0xf140;
	for( u32 i = 0; i < cnt; i++ )
	{
		u8 iv[ 0x10 ] __attribute__(( aligned( 32 ) ));
		bkEntry *e = (bkEntry*)( stuff + pos );
		if( e->magic != 0x03adf17e )
		{
			gprintf( "bad bk magic for entry %u/%u\n", i, cnt );
			return false;
		}
		//gprintf( "file %u/%u\n", i + 1, cnt );
		//gprintf( "magic: %08x\n", e->magic );
		//gprintf( "size : %08x\n", e->size );
		//gprintf( "attr : %08x\n", e->attr );
		//gprintf( "type : %08x\n", e->type );
		//gprintf( "name : \"%.69s\"\n", e->name );

		u32 roundedSize = RU( e->size, 0x40 );

		// check for out of range
		if( pos + 0x80 + roundedSize > len )
		{
			gprintf( "file is out of range\n" );
			return false;
		}
		if( e->type == 1 )// file
		{

			// decrypt
			memcpy( &iv, e->iv, 0x10 );
			// decrypt in chunks.  ES_Decrypt() fails if the buffer is too large, apparently
			u32 done = 0;
			u8* buf = stuff + pos + 0x80;
			while( done < roundedSize )
			{
				u32 toDo = MIN( roundedSize - done, 0x8000 );
				if( (ret = ES_Decrypt( ES_KEY_SDCARD, iv, buf, toDo, buf ) ) < 0 )
				{
					gprintf( "ES_Decrypt( %u ): %i\n", i, ret );
					return false;
				}
				done += toDo;
				buf += toDo;
			}
			files << new FileEntry( stuff + pos + 0x80, e->size, e->attr, e->name );
		}
		else if( e->type == 2 )// directory
		{
			files << new FileEntry( NULL, 0, e->attr, e->name );
		}
		else
		{
			gprintf( "enknown type: %u\n", e->type );
			return false;
		}
		pos += roundedSize + 0x80;
	}
	return true;
}
#endif

/*
  s32 ES_Sign(u8 *source, u32 size, u8 *sig, u8 *certs);
  XBP @ 81595378

  CR:24000000  XER:20000000  CTR:00000000 DSIS:00000000
 DAR:00000000 SRR0:81595378 SRR1:00009032   LR:8159778C
  r0:9297CC3F   r1:92422B40   r2:81669300   r3:92422C00
  r4:00000014   r5:92422C80   r6:9297CC00   r7:DE6B38B7
  r8:6AFFF5AD   r9:86F64029  r10:35B1CB56  r11:92422B20
 r12:8434ED87  r13:8166AD20  r14:92422E00  r15:0000F280
 r16:92422D00  r17:00000000  r18:92403220  r19:00000000
 r20:00000020  r21:00000080  r22:92980C40  r23:00000000
 r24:00000001  r25:9297CC00  r26:00000140  r27:9297CB40
 r28:00000000  r29:00000000  r30:81607E50  r31:00010000


81595378
81597788
8134AE4C
813477B0
8150C0F0


after ES_Sign, they write r5 to the output buffer with length 0x40
then they write the device.cert for 0x180 bytes
then they write the buffer used as input for r6 for 0x180 bytes

then they write the bk header for 0x80 bytes




  */
