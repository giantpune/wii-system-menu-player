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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils/ash.h"
#include "lz77.h"
#include "gecko.h"
#include "U8Archive.h"

U8Archive::U8Archive(const u8 *stuff, u32 len )
	: fst( NULL ),
	  name_table( NULL ),
	  data( NULL )
{
	if( stuff )
	{
		SetData( stuff, len );
	}
}

void U8Archive::SetData( const u8 *stuff, u32 len )
{
	fst = NULL;
	name_table = NULL;
	data = NULL;
	if( !stuff || len < 0x40 )
	{
		gprintf( "SetData(): !stuff || len < 0x40  %p %08x\n", stuff, len );
		return;
	}
	stuff = FindU8Tag( stuff, len );
	if( !stuff )
	{
		gprintf( "U8 tag not found\n" );
		return;
	}
	const U8Header * binHdr = (const U8Header *)stuff;

	const u8* fst_buffer = stuff + binHdr->rootNodeOffset;
	fst = (FstEntry *)fst_buffer;
	u32 name_table_offset = fst->filelen * 0xC;
	name_table = (char *)( fst_buffer + name_table_offset );
	data = (u8*)stuff;
}

const u8 *U8Archive::FindU8Tag( const u8* stuff, u32 len )
{
	if( !stuff || len < 0x20 )
	{
		return NULL;
	}
	if( *(u32*)( stuff ) == 0x55AA382D )
	{
		return stuff;
	}
	if( len > 0x620 && *(u32*)( stuff + 0x600 ) == 0x55AA382D )
	{
		return stuff + 0x600;
	}
	if( len > 0x660 && *(u32*)( stuff + 0x640 ) == 0x55AA382D )
	{
		return stuff + 0x640;
	}
	return NULL;
}

u8 *U8Archive::GetFile( const char *path, u32 *size ) const
{
	if( !path || !fst || !name_table )
	{
		return NULL;
	}

	s32 entryNo = EntryFromPath( path );
	if( entryNo < 1 )
	{
		gprintf( "U8: entry wasn\'t found in the archive  \"%s\"\n", path );
		return NULL;
	}
	if( fst[ entryNo ].filetype )
	{
		gprintf( "U8: \"%s\" is a folder\n", path );
		return NULL;
	}
	if( size )
	{
		*size = fst[ entryNo ].filelen;
	}
	return data + fst[ entryNo ].fileoffset;
}

u8* U8Archive::GetFileAllocated( const char *path, u32 *size ) const
{
	u32 len;
	const u8 *stuff = GetFile( path, &len );
	if( !stuff )
	{
		return NULL;
	}

	// check for IMD5 header and skip it
	if( len > 0x40 && *(u32*)stuff == 0x494d4435 )// IMD5
	{
		stuff += 0x20;
		len -= 0x20;
	}

	u8* ret = NULL;
	// determine if it needs to be decompressed
	if( IsAshCompressed( stuff, len ) )
	{
		//u32 len2 = len;
		// ASH0
		ret = DecompressAsh( stuff, len );
		if( !ret )
		{
			gprintf( "out of memory\n" );
			return NULL;
		}
		/*
		u8* ret2 = DecompressAsh2( stuff, len2 );
		if( !ret2 )
		{
			gprintf( "out of memory 2\n" );
			return NULL;
		}
		if( len != len2 || memcmp( ret, ret2, len ) )
		{
			gprintf( "doesn\'t match\n" );
		}
		else
		{
			gprintf( "matches\n" );
		}
		exit( 0 );*/
	}
	else if( isLZ77compressed( stuff ) )
	{
		// LZ77 with no magic word
		if( decompressLZ77content( stuff, len, &ret, &len ) )
		{
			return NULL;
		}
	}
	else if( *(u32*)( stuff ) == 0x4C5A3737 )// LZ77
	{
		// LZ77 with a magic word
		if( decompressLZ77content( stuff + 4, len - 4, &ret, &len ) )
		{
			return NULL;
		}
	}
	else
	{
		// just copy the data out of the archive
		ret = (u8*)memalign( 32, len );
		if( !ret )
		{
			gprintf( "out of memory\n" );
			return NULL;
		}
		memcpy( ret, stuff, len );
	}
	if( size )
	{
		*size = len;
	}

	// flush the cache so if there are any textures in this data, it will be ready for the GX
	DCFlushRange( ret, len );
	return ret;
}

u32 U8Archive::FileDescriptor( const char *path ) const
{
	int ret = EntryFromPath( path );
	if( ret < 1 )
	{
		return 0;
	}
	return ret;
}

u8* U8Archive::GetFileFromFd( u32 fd, u32 *size )const
{
	if( !fst || !name_table || fd >= fst[ 0 ].filelen )
	{
		return NULL;
	}
	if( fst[ fd ].filetype )
	{
		gprintf( "U8: \"%s\" is a folder\n", FstName( &fst[ fd ] ) );
		return NULL;
	}
	if( size )
	{
		*size = fst[ fd ].filelen;
	}
	return data + fst[ fd ].fileoffset;
}

char *U8Archive::FstName( const FstEntry *entry ) const
{
	if( entry == &fst[ 0 ] )
	{
		return NULL;
	}

	return (char*)name_table + ( *((u32 *)entry) & 0x00ffffff );
}

int U8Archive::strcasecmp_slash( const char *s1, const char *s2 )
{
	while( toupper( *s1 ) == toupper( *s2++ ) )
	{
		if( *s1++ == 0 || *s1 == '/' )
		{
			return 0;
		}
	}

	return( *(const unsigned char*)s1 - *(const unsigned char *)( s2 - 1 ) );
}

int U8Archive::strlen_slash( const char *s )
{
	int ret = 0;
	while( *s && *s++ != '/' )
	{
		ret++;
	}

	return ret;
}

u32 U8Archive::NextEntryInFolder( u32 current, u32 directory ) const
{
	u32 next = ( fst[ current ].filetype ? fst[ current ].filelen : current + 1 );
	if( next < fst[ directory ].filelen )
		return next;

	return 0;
}

s32 U8Archive::EntryFromPath( const char *path, int d ) const
{
	//gprintf( "EntryFromPath( \"%s\", %d )\n", path, d );
	while( *path == '/' )
	{
		path++;
	}

	if( !fst[ d ].filetype )
	{
		gprintf("ERROR!!  %s is not a directory\n", FstName( &fst[ d ] ) );
		return -1;
	}

	u32 next = d + 1;

	FstEntry *entry = &fst[ next ];

	while( next )
	{
		//does this entry match.
		//strlen_slash is used because if looking for "dvd:/gameboy/" it would return a false positive if it hit "dvd:/gameboy advance/" first
		if( !strcasecmp_slash( path, FstName( entry ) ) && ( strlen( FstName( entry ) ) == (u32)strlen_slash( path ) ) )
		{
			char *slash = strchr( path, '/' );
			if( slash && *( slash + 1 ) )
			{
				//we are looking for a file somewhere in this folder
				return EntryFromPath( slash + 1, next );
			}
			//this is the actual entry we were looking for
			return next;
		}

		//find the next entry in this folder
		next = NextEntryInFolder( next, d );
		entry = &fst[ next ];
	}

	//no entry with the given path was found
	return -1;
}

U8NandArchive::U8NandArchive( const char* nandPath )
	: U8Archive( NULL, 0 ),
	  fd( -1 ),
	  dataOffset( 0 )
{
	if( nandPath )
	{
		SetFile( nandPath );
	}
}

U8NandArchive::~U8NandArchive()
{
	if( fd >= 0 )
	{
		ISFS_Close( fd );
	}
	free( fst );
}

bool U8NandArchive::SetFile( const char* nandPath )
{
	FREE( fst );
	FREE( name_table );
	CloseFile();

	// open file
	if( (fd = ISFS_Open( nandPath, ISFS_OPEN_READ ) ) < 0 )
	{
		gprintf( "U8NandArchive:  ISFS_Open( \"%s\" ) failed\n", nandPath );
		return false;
	}

	// get file size
	fstats stats __attribute__(( aligned( 32 ) ));
	int ret = ISFS_GetFileStats( fd, &stats );
	if( ret < 0 )
	{
		CloseFile();
		gprintf( "U8NandArchive:  ISFS_GetFileStats( \"%s\" ) failed\n", nandPath );
		return false;
	}

	// buffer for reading the header and stuff
	u8* buffer = (u8*)memalign( 32, 0x800 );
	if( !buffer )
	{
		CloseFile();
		gprintf( "U8NandArchive: enomem\n" );
		return false;
	}

	// read a chunk big enough that it should contain the U8 header if there is going to be one
	if( (ret = ISFS_Read( fd, buffer, 0x800 )) != 0x800 )
	{
		free( buffer );
		CloseFile();
		gprintf( "U8NandArchive: ISFS_Read( 0x800 ) = %i\n", ret );
		return false;
	}

	// find the start of the U8 data
	U8Header* tagStart = (U8Header*)FindU8Tag( buffer, ret );
	if( !tagStart )
	{
		free( buffer );
		CloseFile();
		gprintf( "U8NandArchive: didn't see a U8 tag\n" );
		return false;
	}

	// remember where in the file the U8 starts
	dataOffset = ( (u8*)tagStart - buffer );

	// allocate memory and read the fst
	if( !(fst = (FstEntry *)memalign( 32, RU( tagStart->dataOffset - dataOffset, 32 ) ) )
			|| ( ISFS_Seek( fd, dataOffset + tagStart->rootNodeOffset, SEEK_SET ) != (s32)( dataOffset + tagStart->rootNodeOffset ) )
			|| ( ISFS_Read( fd, fst, tagStart->dataOffset - dataOffset ) != (s32)( tagStart->dataOffset - dataOffset ) )
			|| ( fst->filelen * 0xC > tagStart->dataOffset ) )
	{
		dataOffset = 0;
		free( buffer );
		FREE( fst );
		CloseFile();
		gprintf( "U8NandArchive: error reading fst\n" );
		return false;
	}

	// set name table pointer
	u32 name_table_offset = fst->filelen * 0xC;
	name_table = ((char *)fst) + name_table_offset;

	free( buffer );
	return true;
}

u8* U8NandArchive::GetFileAllocated( const char *path, u32 *size ) const
{
	//gprintf( "U8NandArchive::GetFileAllocated( %s )\n" );
	if( !path || !fst )
	{
		return NULL;
	}

	// find file
	int f = EntryFromPath( path, 0 );
	if( f < 1 || f >= (int)fst[ 0 ].filelen )
	{
		gprintf( "U8: \"%s\" wasn't found in the archive.\n", path );
		return NULL;
	}
	if( fst[ f ].filetype )
	{
		gprintf( "U8: \"%s\" is a folder\n", path );
		return NULL;
	}

	// create a buffer
	u8* ret = (u8*)memalign( 32, RU( fst[ f ].filelen, 32 ) );
	if( !ret )
	{
		gprintf( "U8: out of memory\n" );
		return NULL;
	}

	// seek and read
	if( ISFS_Seek( fd, dataOffset + fst[ f ].fileoffset, SEEK_SET ) != (s32)( dataOffset + fst[ f ].fileoffset )
			|| ISFS_Read( fd, ret, fst[ f ].filelen ) != (s32)fst[ f ].filelen )
	{
		free( ret );
		gprintf( "U8: error reading data from nand\n" );
		gprintf( "fd: %i  fst[ fd ].filelen: %08x\n", fd, fst[ f ].filelen );
		return NULL;
	}

	u32 len = fst[ f ].filelen;
	u8* ret2;
	// determine if it needs to be decompressed
	if( IsAshCompressed( ret, len ) )
	{
		// ASH0
		ret2 = DecompressAsh( ret, len );
		if( !ret2 )
		{
			free( ret );
			gprintf( "out of memory\n" );
			return NULL;
		}
		free( ret );

	}
	else if( isLZ77compressed( ret ) )
	{
		// LZ77 with no magic word
		if( decompressLZ77content( ret, len, &ret2, &len ) )
		{
			free( ret );
			return NULL;
		}
		free( ret );
	}
	else if( *(u32*)( ret ) == 0x4C5A3737 )// LZ77
	{
		// LZ77 with a magic word
		if( decompressLZ77content( ret + 4, len - 4, &ret2, &len ) )
		{
			free( ret );
			return NULL;
		}
		free( ret );
	}
	else
	{
		// already got what we are after
		ret2 = ret;
	}

	if( size )
	{
		*size = len;
	}

	// flush the cache so if there are any textures in this data, it will be ready for the GX
	DCFlushRange( ret2, len );
	return ret2;
}



U8FileArchive::U8FileArchive( const char* filePath )
	: U8Archive( NULL, 0 ),
	  fd( NULL ),
	  dataOffset( 0 )
{
	if( filePath )
	{
		SetFile( filePath );
	}
}

U8FileArchive::~U8FileArchive()
{
	if( fd )
	{
		fclose( fd );
	}
	free( fst );
}

bool U8FileArchive::SetFile( const char* filePath )
{
	FREE( fst );
	FREE( name_table );
	CloseFile();

	// open file
	if( !(fd = fopen( filePath, "rb" ) ) )
	{
		gprintf( "U8FileArchive:  fopen( \"%s\" ) failed\n",filePath );
		return false;
	}

	int ret;

	// buffer for reading the header and stuff
	u8* buffer = (u8*)memalign( 32, 0x800 );
	if( !buffer )
	{
		CloseFile();
		gprintf( "U8FileArchive: enomem\n" );
		return false;
	}

	// read a chunk big enough that it should contain the U8 header if there is going to be one
	if( (ret = fread( buffer, 0x800, 1, fd )) != 1 )
	{
		free( buffer );
		CloseFile();
		gprintf( "U8FileArchive: fread( 0x800 ) = %i\n", ret );
		return false;
	}

	// find the start of the U8 data
	U8Header* tagStart = (U8Header*)FindU8Tag( buffer, 0x800 );
	if( !tagStart )
	{
		free( buffer );
		CloseFile();
		gprintf( "U8FileArchive: didn't see a U8 tag\n" );
		return false;
	}

	// remember where in the file the U8 starts
	dataOffset = ( (u8*)tagStart - buffer );

	// allocate memory and read the fst
	if( !(fst = (FstEntry *)memalign( 32, RU( tagStart->dataOffset - dataOffset, 32 ) ) )
			|| ( fseek( fd, dataOffset + tagStart->rootNodeOffset, SEEK_SET ) )
			|| ( fread( fst, tagStart->dataOffset - dataOffset, 1, fd ) != 1 )
			|| ( fst->filelen * 0xC > tagStart->dataOffset ) )
	{
		gprintf( "U8FileArchive: error reading fst\n" );
		gprintf( "%08x, %p, %p, %08x\n", dataOffset, buffer, fst, ftell( fd ) );
		dataOffset = 0;
		free( buffer );
		FREE( fst );
		CloseFile();
		return false;
	}

	// set name table pointer
	u32 name_table_offset = fst->filelen * 0xC;
	name_table = ((char *)fst) + name_table_offset;

	free( buffer );
	return true;
}

u8* U8FileArchive::GetFileAllocated( const char *path, u32 *size ) const
{
	//gprintf( "U8FileArchive::GetFileAllocated( %s )\n" );
	if( !path || !fst )
	{
		return NULL;
	}

	// find file
	int f = EntryFromPath( path, 0 );
	if( f < 1 || f >= (int)fst[ 0 ].filelen )
	{
		gprintf( "U8: \"%s\" wasn't found in the archive.\n", path );
		return NULL;
	}
	if( fst[ f ].filetype )
	{
		gprintf( "U8: \"%s\" is a folder\n", path );
		return NULL;
	}

	// create a buffer
	u8* ret = (u8*)memalign( 32, RU( fst[ f ].filelen, 32 ) );
	if( !ret )
	{
		gprintf( "U8: out of memory\n" );
		return NULL;
	}

	// seek and read
	if( fseek( fd, dataOffset + fst[ f ].fileoffset, SEEK_SET )
			|| fread( ret, fst[ f ].filelen, 1, fd ) != 1 )
	{
		free( ret );
		gprintf( "U8: error reading data from nand\n" );
		gprintf( "fd: %i  fst[ fd ].filelen: %08x\n", f, fst[ f ].filelen );
		return NULL;
	}

	u32 len = fst[ f ].filelen;
	u8* ret2;
	// determine if it needs to be decompressed
	if( IsAshCompressed( ret, len ) )
	{
		// ASH0
		ret2 = DecompressAsh( ret, len );
		if( !ret2 )
		{
			free( ret );
			gprintf( "out of memory\n" );
			return NULL;
		}
		free( ret );
	}
	else if( isLZ77compressed( ret ) )
	{
		// LZ77 with no magic word
		if( decompressLZ77content( ret, len, &ret2, &len ) )
		{
			free( ret );
			return NULL;
		}
		free( ret );
	}
	else if( *(u32*)( ret ) == 0x4C5A3737 )// LZ77
	{
		// LZ77 with a magic word
		if( decompressLZ77content( ret + 4, len - 4, &ret2, &len ) )
		{
			free( ret );
			return NULL;
		}
		free( ret );
	}
	else
	{
		// already got what we are after
		ret2 = ret;
	}

	if( size )
	{
		*size = len;
	}

	// flush the cache so if there are any textures in this data, it will be ready for the GX
	DCFlushRange( ret2, len );
	return ret2;
}

