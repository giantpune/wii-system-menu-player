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
#ifndef U8ARCHIVE_H
#define U8ARCHIVE_H

#include <string>
#include "tools.h"

class U8Archive
{
public:
	U8Archive( const u8 *stuff, u32 len );

	// set the data used for this archive
	virtual void SetData( const u8 *stuff, u32 len );

	// returns a pointer to a file within the archive
	virtual u8* GetFile( const char *path, u32 *size = NULL ) const;
	inline u8* GetFile( const std::string &path, u32 *size = NULL ) const
	{
		return GetFile( path.c_str(), size );
	}

	// gets a file and copies it into a newly memalign()'d buffer
	//! if the data looks ASH or LZ77 compressed, it is decompressed
	virtual u8* GetFileAllocated( const char *path, u32 *size = NULL ) const;
	inline u8* GetFileAllocated( const std::string &path, u32 *size ) const
	{
		return GetFileAllocated( path.c_str(), size );
	}

	virtual u32 FileDescriptor( const char *path ) const;
	inline u32 FileDescriptor( const std::string &path ) const{ return FileDescriptor( path.c_str() ); }
	virtual u8* GetFileFromFd( u32 fd, u32 *size = NULL )const;


protected:
	struct U8Header
	{
		u32 magic;
		u32 rootNodeOffset;
		u32 headerSize;
		u32 dataOffset;
		u8 zeroes[16];
	} __attribute__((packed));

	struct FstEntry
	{
		u8 filetype;
		char name_offset[3];
		u32 fileoffset;
		u32 filelen;
	} __attribute__((packed));

	FstEntry *fst;
	char *name_table;
	u8* data;

	u32 NextEntryInFolder( u32 current, u32 directory ) const;
	s32 EntryFromPath( const char *path, int d = 0 ) const;
	char *FstName( const FstEntry *entry ) const __attribute__ ((pure));

	static int strlen_slash( const char *s )__attribute__ ((pure));

	static int strcasecmp_slash( const char *s1, const char *s2 )__attribute__ ((pure));

	// inline toupper for strcasecmp - U8 archives are case-insinsitive
	static char toupper( char c )
	{
		if( c <= 'z' && c >= 'a' )
			return c - 0x20;
		return c;
	}

	// looks in some common offsets for a U8 tag
	static const u8* FindU8Tag( const u8* stuff, u32 len );
};

// class to access files from an archive that is saved on the nand
// GetFile() will return NULL, use GetfileAllocated()
class U8NandArchive : public U8Archive
{
public:
	U8NandArchive( const char* nandPath );
	~U8NandArchive();

	bool SetFile( const char* nandPath );


	// not implimented in this subclass...
	void SetData( const u8 *stuff, u32 len ){}
	u8* GetFile( const char *path, u32 *size = NULL ) const { return NULL; }
	u8* GetFileFromFd( u32 fd, u32 *size = NULL )const { return NULL; }


	// gets a file and copies it into a newly memalign()'d buffer
	//! if the data looks ASH or LZ77 compressed, it is decompressed
	u8* GetFileAllocated( const char *path, u32 *size = NULL ) const;

private:
	s32 fd;

	// where the U8 header starts within the file
	u32 dataOffset;
	void CloseFile()
	{
		if( fd >= 0 )
		{
			ISFS_Close( fd );
			fd = -1;
		}
	}
};

#endif // U8ARCHIVE_H
