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
#include <cstring>
#include <malloc.h>

#include <ogc/machine/processor.h>

#include "contentbin.h"
#include "databin.h"
#include "gecko.h"
#include "md5.h"
#include "sdcontentcrypto.h"
#include "sha1.h"
#include "tools.h"

ContentBin::ContentBin()
{
}

const u8 iconOnlyU8Header[ 0x60 ] =
{
	0x55, 0xaa, 0x38, 0x2d, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x01, 0x88, 0x00, 0x6d, 0x65, 0x74, 0x61, 0x00, 0x69, 0x63, 0x6f, 0x6e, 0x2e, 0x62,
	0x69, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

u8* ContentBin::GetIconAsBannerData( FILE* file, u32 pathTID, u32 &outSize, u32 &installedBytes )
{
	int ret;
	u8* retB = NULL;
	u8* iconBuff = NULL;
	u32 outLen = 0;
	u32 iconSize = 0;
	Header *header = NULL;
	BkHeader bkHeader;


	u8 md5Actual[ 0x10 ];
	u8 md5Header[ 0x10 ];

	//! ipl::NandSDWorker::c_md5_secret_key
	u8 md5Blacker[ 0x10 ] = { 0x0E, 0x65, 0x37, 0x81, 0x99, 0xBE, 0x45, 0x17, 0xAB, 0x06, 0xEC, 0x22, 0x45, 0x1A, 0x57, 0x93 };

	// remember the position
	int pos = ftell( file );

	// seek to the beginning
	rewind( file );

	// read encrypted imet header
	u8* imetBuf = (u8*)memalign( 32, 0x640 );
	if( !imetBuf )
	{
		gprintf( "ContentBin::GetIconAsBannerData(): out of memory\n" );
		goto done;
	}
	if( (ret = fread( imetBuf, 0x640, 1, file ) ) != 1 )
	{
		gprintf( "ContentBin::GetIconAsBannerData(): short read: %08x\n", ret );
		goto done;
	}

	// decrypt header
	if( (ret = DecryptWithSDKey( imetBuf, 0x640, imetBuf ) ) < 0 )
	{
		gprintf( "ES_Decrypt(): %i\n", ret );
		goto done;
	}
	header = (Header*)imetBuf;
	if( (header->tid & 0xffffffff ) != pathTID )
	{
		gprintf( "ContentBin::GetIconAsBannerData(): header TID doesn\'t match: %.4s %.4s\n", ((char*)imetBuf) + 4, &pathTID );
		goto done;
	}

	// verify MD5
	memcpy( md5Header, header->md5Iheader, 0x10 );
	memcpy( header->md5Iheader, md5Blacker, 0x10 );
	MD5::GetMD5( imetBuf, 0x640, md5Actual );
	if( memcmp( md5Actual, md5Header, 0x10 ) )
	{
		gprintf( "ContentBin::LoadFromMemory(): md5Iheader mismatch\n" );
		goto done;
	}

	iconSize = RU( header->iconSize, 0x40 );

	// read encrypted icon.bin
	iconBuff = (u8*)memalign( 32, iconSize );
	if( !iconBuff )
	{
		gprintf( "ContentBin::GetIconAsBannerData(): out of memory\n" );
		goto done;
	}
	if( (ret = fread( iconBuff, iconSize, 1, file ) ) != 1 )
	{
		gprintf( "ContentBin::GetIconAsBannerData(): short read: %08x\n", ret );
		goto done;
	}
	// decrypt icon
	if( (ret = DecryptWithSDKey( iconBuff, iconSize, iconBuff ) ) < 0 )
	{
		gprintf( "ES_Decrypt(): %i\n", ret );
		goto done;
	}

	// check md5 against the one in the file header
	MD5::GetMD5( iconBuff, iconSize, md5Actual );
	if( memcmp( md5Actual, header->md5Icon, 0x10 ) )
	{
		gprintf( "DataBin::LoadFromMemory(): md5Icon mismatch\n" );
		goto done;
	}

	// create output buffer
	outLen = 0x640 + sizeof( iconOnlyU8Header ) + iconSize;
	retB = (u8*)memalign( 32, outLen );
	if( !retB )
	{
		gprintf( "ContentBin::GetIconAsBannerData(): out of memory\n" );
		goto done;
	}
	// generate a u8 archive for passing to our existing banner functions
	memcpy( retB, imetBuf, 0x640 );
	memcpy( retB + 0x640, iconOnlyU8Header, sizeof( iconOnlyU8Header ) );
	memcpy( retB + 0x640 + sizeof( iconOnlyU8Header ), iconBuff, iconSize );

	// get file size from the bk header
	fseek( file, 0x640 + iconSize, SEEK_SET );
	if( fread( &bkHeader, sizeof( BkHeader ), 1, file ) != 1 )
	{
		gprintf( "short read for content.bin\n" );
		goto done;
	}
	if( bkHeader.magic != 0x426b0001 )
	{
		gprintf( "bad bk header for content.bin\n" );
		goto done;
	}

	installedBytes = bkHeader.payloadSize;

done:
	outSize = outLen;

	// seek back to the original position
	fseek( file, pos, SEEK_SET );

	// free temp imet and icon buffers
	free( imetBuf );
	free( iconBuff );

	return retB;
}

u32 ContentBin::GetInstalledSize( FILE *file )
{
	int ret;
	u32 retN = 0;
	u32 iconSize = 0;
	Header *header = NULL;
	BkHeader bkHeader;


	// remember the position
	int pos = ftell( file );

	// seek to the beginning
	rewind( file );

	// read encrypted imet header
	u8* imetBuf = (u8*)memalign( 32, 0x640 );
	if( !imetBuf )
	{
		gprintf( "ContentBin::GetInstalledSize(): out of memory\n" );
		goto done;
	}
	if( (ret = fread( imetBuf, 0x640, 1, file ) ) != 1 )
	{
		gprintf( "ContentBin::GetInstalledSize(): short read: %08x\n", ret );
		goto done;
	}

	// decrypt header
	if( (ret = DecryptWithSDKey( imetBuf, 0x640, imetBuf ) ) < 0 )
	{
		gprintf( "ES_Decrypt(): %i\n", ret );
		goto done;
	}
	header = (Header*)imetBuf;
	iconSize = RU( header->iconSize, 0x40 );


	// get file size from the bk header
	fseek( file, 0x640 + iconSize, SEEK_SET );
	if( fread( &bkHeader, sizeof( BkHeader ), 1, file ) != 1 )
	{
		gprintf( "short read for content.bin\n" );
		goto done;
	}
	if( bkHeader.magic != 0x426b0001 )
	{
		gprintf( "bad bk header for content.bin\n" );
		goto done;
	}

	retN = bkHeader.payloadSize;

done:

	// seek back to the original position
	fseek( file, pos, SEEK_SET );

	free( imetBuf );

	return retN;
}

#ifndef SLIM_CONTENT_BIN
bool ContentBin::InstallChannel( u8* stuff, u32 len, u32 pathTID )
{
	if( len < 0x640 + 0x80 )//header + bkHeader size
	{
		gprintf( "len is too small: %08x\n", len );
		return false;
	}
	int ret;

	// decrypt header
	if( (ret = DecryptWithSDKey( stuff, 0x640, stuff ) ) < 0 )
	{
		gprintf( "ES_Decrypt(): %i\n", ret );
		return false;
	}
	Header *header = (Header*)stuff;
	//hexdump( header, 0x640 );

	// verify MD5
	u8 md5Actual[ 0x10 ];
	u8 md5Header[ 0x10 ];

	//! ipl::NandSDWorker::c_md5_secret_key
	u8 md5Blacker[ 0x10 ] = { 0x0E, 0x65, 0x37, 0x81, 0x99, 0xBE, 0x45, 0x17, 0xAB, 0x06, 0xEC, 0x22, 0x45, 0x1A, 0x57, 0x93 };

	memcpy( md5Header, header->md5Iheader, 0x10 );
	memcpy( header->md5Iheader, md5Blacker, 0x10 );
	MD5::GetMD5( stuff, 0x640, md5Actual );
	if( memcmp( md5Actual, md5Header, 0x10 ) )
	{
		gprintf( "ContentBin::LoadFromMemory(): md5Iheader mismatch\n" );
		return false;
	}
	gprintf( "md5Iheader matches\n" );

	u32 iconSize = RU( header->iconSize, 0x40 );
	if( len < 0x640 + 0x80 + iconSize )//header + bkHeader size + iconSize
	{
		gprintf( "len is too small (2): %08x\n", len );
		return false;
	}

	// decrypt icon
	u8* iconBuff = stuff + 0x640;
	if( (ret = DecryptWithSDKey( iconBuff, iconSize, iconBuff ) ) < 0 )
	{
		gprintf( "ES_Decrypt(): %i\n", ret );
		return false;
	}

	// check md5 against the one in the file header
	MD5::GetMD5( iconBuff, iconSize, md5Actual );
	if( memcmp( md5Actual, header->md5Icon, 0x10 ) )
	{
		gprintf( "DataBin::LoadFromMemory(): md5Icon mismatch\n" );
		return false;
	}

	// theres also an IMD5 header here with yet another MD5 for the icon.  maybe we can check it one of these days

	// do bk header
	BkHeader *bkHeader = (BkHeader*)( iconBuff + iconSize );
	if( !VerifyBKWad( iconBuff + iconSize, ( len - 0x640 ) - iconSize ) )
	{
		return false;
	}


	u32 deviceID __attribute__(( aligned( 32 ) ));
	ES_GetDeviceID( &deviceID );
	if( bkHeader->deviceID != deviceID )
	{
		gprintf( "doesn't belong to this wii\n" );
		return false;
	}
	// get tmd
	u32 tmdSize = RU( bkHeader->tmdSize, 0x40 );
	if( len < 0x640 + 0x80 + iconSize + tmdSize )//header + bkHeader size + iconSize + tmdSize
	{
		gprintf( "len is too small (3): %08x\n", len );
		return false;
	}
	signed_blob *s_tmd = (signed_blob *)( stuff + 0x640 + 0x80 + iconSize );
	tmd *t = (tmd*)SIGNATURE_PAYLOAD( s_tmd );

	if( t->title_id != header->tid )
	{
		gprintf( "tid doesn\'t match  %016llx  %016llx\n", t->title_id, header->tid );
		return false;
	}
	if( t->num_contents > MAX_NUM_TMD_CONTENTS )
	{
		gprintf( "tid is busted\n" );
		return false;
	}

	if( (ret = ES_AddTitleTMD( s_tmd, bkHeader->tmdSize ) < 0 ) )
	{
		gprintf( "ES_AddTitleTMD( %p ): %i\n", s_tmd, ret );
		return false;
	}

	// do tmd contents
	tmd_content *contents = TMD_CONTENTS( t );

	u8* cData = stuff + 0x640 + 0x80 + iconSize + tmdSize;
	for( u16 i = 0; i < t->num_contents; i++ )
	{
		// skip contents that arenn't here
		if( !( bkHeader->includedContents[ i / 8 ] & ( 1 << ( i % 8 ) ) ) )
		{
			continue;
		}
		s32 fd;
		u32 len = RU( contents[ i ].size, 0x40 );

		if( (fd = ES_AddContentStart( header->tid, contents[ i ].cid ) ) < 0 )
		{
			gprintf( "ES_AddContentStart( %08x ): %i\n", contents[ i ].cid, fd );
			goto fail;
		}

		if( (ret = ES_AddContentData( fd, cData, len ) ) < 0 )
		{
			gprintf( "ES_AddContentData( %08x ): %i\n", fd, ret );
			goto fail;
		}

		if( (ret = ES_AddContentFinish( fd ) ) < 0 )
		{
			gprintf( "ES_AddContentFinish( %08x ): %i\n", fd, ret );
			goto fail;
		}

		// skip to next content
		cData += len;
	}

	if( (ret = ES_AddTitleFinish() ) < 0 )
	{
		gprintf( "ES_AddTitleFinish(): %i\n", ret );
		goto fail;
	}
	//gprintf( "success\n" );
	return true;

fail:
	gprintf( "failing...\n" );
	ES_AddTitleCancel();
	return false;
}
#endif


