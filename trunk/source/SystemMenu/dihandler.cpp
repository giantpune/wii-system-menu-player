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
#include <unistd.h>
#include "dihandler.h"
#include "gecko.h"
#include "tools.h"

DiHandler *DiHandler::instance = NULL;
bool DiHandler::threadSleep = true;
bool DiHandler::threadExit = false;
lwp_t DiHandler::thread = LWP_THREAD_NULL;
mutex_t DiHandler::mutex = LWP_THREAD_NULL;

s32 WDVD_Close(void);

DiHandler::DiHandler()
{
	LWP_MutexInit( &mutex, false );
	LWP_CreateThread( &thread, ThreadMain, NULL, NULL, 32768, 80);
}

void DiHandler::Wake()
{
	threadSleep = false;
	LWP_ResumeThread( thread );
}

void DiHandler::Sleep()
{
	threadSleep = true;
	LWP_SuspendThread( thread );
}

void DeInit()
{
	WDVD_Close();
}

#define IOCTL_DI_READID		0x70
#define IOCTL_DI_READ		0x71
#define IOCTL_DI_GETCOVER	0x88
#define IOCTL_DI_RESET		0x8A
#define IOCTL_DI_OPENPART	0x8B
#define IOCTL_DI_CLOSEPART	0x8C
#define IOCTL_DI_UNENCREAD	0x8D
#define PTABLE_OFFSET	0x40000

static u32 inbuf[ 8 ]  ATTRIBUTE_ALIGN( 32 );
static u32 outbuf[ 8 ] ATTRIBUTE_ALIGN( 32 );
static u32 wdvd_buffer[ 0x5000 / 4 ] __attribute__(( aligned( 32 ) ));

static const char di_fs[] ATTRIBUTE_ALIGN( 32 ) = "/dev/di";
static s32 di_fd = -1;

//bool partitionOpen = false;

s32 WDVD_Init(void)
{
	if (di_fd < 0)
	{
		di_fd = IOS_Open( di_fs, 0 );
		if (di_fd < 0)
			return di_fd;
	}
	//gprintf("WDVD_Init() ok di_fd: %d\n", di_fd );

	return 0;
}

s32 WDVD_Close(void)
{
	if (di_fd >= 0)
	{
		IOS_Close( di_fd );
		di_fd = -1;
	}

	return 0;
}

s32 WDVD_Reset(void)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_RESET << 24;
	inbuf[1] = 1;

	ret = IOS_Ioctl(di_fd, IOCTL_DI_RESET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0)
		return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_GetCoverStatus( u32 *status )
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_GETCOVER << 24;

	ret = IOS_Ioctl(di_fd, IOCTL_DI_GETCOVER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0)
		return ret;

	if (ret == 1)
	{
		*status = outbuf[ 0 ];
		return 0;
	}

	return -ret;
}

s32 WDVD_ReadDiskId(void *id) {
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_READID << 24;

	ret = IOS_Ioctl(di_fd, IOCTL_DI_READID, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0)
		return ret;

	if (ret == 1) {
		memcpy(id, outbuf, sizeof(dvddiskid));
		return 0;
	}

	return -ret;
}

s32 WDVD_ClosePartition(void)
{
	s32 ret;

//	partitionOpen = false;

	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_CLOSEPART << 24;

	ret = IOS_Ioctl(di_fd, IOCTL_DI_CLOSEPART, inbuf, sizeof(inbuf), NULL, 0);
	if (ret < 0)
		return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_UnencryptedRead(void *buf, u32 len, u64 offset) {
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_UNENCREAD << 24;
	inbuf[1] = len;
	inbuf[2] = (u32)(offset >> 2);

	ret = IOS_Ioctl(di_fd, IOCTL_DI_UNENCREAD, inbuf, sizeof(inbuf), buf, len);
	if (ret < 0)
		return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_Read(void *buf, u32 len, u64 offset) {
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_READ << 24;
	inbuf[1] = len;
	inbuf[2] = (u32)(offset >> 2);

	ret = IOS_Ioctl(di_fd, IOCTL_DI_READ, inbuf, sizeof(inbuf), buf, len);
	if (ret < 0)
		return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_FindPartition( u64 *outbuf )
{
	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	u64 offset = 0, table_offset = 0;

	u32 cnt, nb_partitions;
	s32 ret;
	//u32 *buffer = NULL;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	//buffer = (u32 *)memalign(32, 0x20);
	//if (!buffer)
	//	return -1;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	ret = WDVD_UnencryptedRead( wdvd_buffer, 0x20, PTABLE_OFFSET );
	if (ret < 0)
	{
		//free( buffer );
		return ret;
	}

	nb_partitions = wdvd_buffer[0];
	table_offset  = wdvd_buffer[1] << 2;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	ret = WDVD_UnencryptedRead( wdvd_buffer, 0x20, table_offset );
	if (ret < 0)
	{
//		free( buffer );
		return ret;
	}

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	for (cnt = 0; cnt < nb_partitions; cnt++) {
		u32 type = wdvd_buffer[cnt * 2 + 1];

		if (!type)
		{
			offset = wdvd_buffer[cnt * 2] << 2;
			break;
		}
	}

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	//free( buffer );

	if (!offset)
		return -1;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	*outbuf = offset;

	return 0;
}

s32 WDVD_OpenPartition(u64 offset) {
	u8 *vector = NULL;

	//u32 *buffer = NULL;
	s32 ret;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	//buffer = (u32 *)memalign(32, 0x5000);
//	if (!buffer)
	//	return -1;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	vector = (u8 *)wdvd_buffer;

	memset(wdvd_buffer, 0, 0x5000);

	wdvd_buffer[0] = (u32)(wdvd_buffer + 0x10);
	wdvd_buffer[1] = 0x20;
	wdvd_buffer[3] = 0x024A;
	wdvd_buffer[6] = (u32)(wdvd_buffer + 0x380);
	wdvd_buffer[7] = 0x49E4;
	wdvd_buffer[8] = (u32)(wdvd_buffer + 0x360);
	wdvd_buffer[9] = 0x20;

	wdvd_buffer[(0x40 >> 2)]     = IOCTL_DI_OPENPART << 24;
	wdvd_buffer[(0x40 >> 2) + 1] = offset >> 2;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	ret = IOS_Ioctlv(di_fd, IOCTL_DI_OPENPART, 3, 2, (ioctlv *)vector);

	//free(buffer);

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	if (ret < 0)
		return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_OpenDataPartition()
{
	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	u64 dataPartitionOffset;
//	u32 status = 0;

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	s32 ret = WDVD_FindPartition( &dataPartitionOffset );
	if( ret < 0 )
	{
		gprintf("WDVD_FindPartition(): %d\n", ret );
		return ret;
	}

	//gprintf( "%s %i\n", __FILE__, __LINE__ );
	ret = WDVD_OpenPartition( dataPartitionOffset );
	if( ret < 0 )
	{
		gprintf("WDVD_OpenPartition( %016llx ): %d\n", dataPartitionOffset, ret );
		return ret;
	}

   // partitionOpen = true;

	//gprintf("opened partition @ %016llx\n", dataPartitionOffset );

	return 0;
}

struct FST_INFO
{
	u32 dol_offset;
	u32 fst_offset;
	u32 fst_size;
	u32 fst_size2;

	u32 pad[ 4 ];// add padding to make it a nice round 0x20 bytes
} __attribute__ ((packed));

struct FST_ENTRY
{
	u8 filetype;
	char name_offset[3];
	u32 fileoffset;
	u32 filelen;
} __attribute__((packed));

static u8 *fst_buffer = NULL;
static char *name_table = NULL;
static FST_ENTRY *fst = NULL;

const char *FstName( const FST_ENTRY *entry )
{
	if( entry == &fst[ 0 ] )
		return NULL;

	return (char*)name_table + ( *((u32 *)entry) & 0x00ffffff );
}

static int strcmp_slash( const char *s1, const char *s2 )
{
	while( *s1 == *s2++ )
		if( *s1++ == 0 || *s1 == '/' )
			return( 0 );

	return( *(const unsigned char*)s1 - *(const unsigned char *)( s2 - 1 ) );
}

static int strlen_slash( const char *s )__attribute__(( pure ));
static int strlen_slash( const char *s )
{
	int ret = 0;
	while( *s && *s++ != '/' )
		ret++;

	return ret;
}

static u32 NextEntryInFolder( u32 current, u32 directory )
{
	u32 next = ( fst[ current ].filetype ? fst[ current ].filelen : current + 1 );
	if( next < fst[ directory ].filelen )
		return next;

	return 0;
}

static s32 EntryFromPath( const char *path, int d )
{
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

	FST_ENTRY *entry = &fst[ next ];

	while( next )
	{
		//gprintf( "\tcomparing: [ %u ] %s\n", next, FstName( entry ) );
		//does this entry match.
		//strlen_slash is used because if looking for "dvd:/gameboy/" it would return a false positive if it hit "dvd:/gameboy advance/" first
		if( !strcmp_slash( path, FstName( entry ) ) && ( strlen( FstName( entry ) ) == (u32)strlen_slash( path ) ) )
		{
			char *slash = strchr( path, '/' );
			if( slash && strlen( slash + 1 ) )
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

void *DiHandler::ThreadMain( void *arg )
{
	enum State
	{
		St_Init,
		St_Reset,
		St_WaitForDisc,
		St_CheckDiscType,
		St_OpenPartition,
		St_WaitForDiscEject,	// some error happened, dont do anything until the current disc is ejected
		St_Idle
	};

	State state = St_Init;

	u32 coverState = 0;

	while( !threadExit )
	{
		usleep( 1000 );

		if( threadSleep )
			LWP_SuspendThread( thread );

		if( state == St_Init )
		{
			if( WDVD_Init() )
			{
				instance->ErrorHappened( E_Init, true );
				threadExit = true;
			}
			state = St_Reset;
			continue;
		}
		else if( state == St_Reset )
		{
			if( WDVD_Reset() )
			{
				instance->ErrorHappened( E_Init, false );
				continue;
			}
			state = St_WaitForDisc;
		}

		// check for disc
		u32 cover = 0;
		if( WDVD_GetCoverStatus( &cover ) )
		{
			gprintf( "WDVD_GetCoverStatus() failed\n" );
			WDVD_Close();
			state = St_Init;
			continue;
		}

		// check if disc status is changed
		if( cover != coverState )
		{
			//gprintf( "cover status: %08x %08x\n", cover, coverState );
			if( cover & 2 )// disc is present and wasnt before
			{
				//gprintf( "disc inserted\n" );
				instance->StartingToReadDisc();
				WDVD_Reset();
				state = St_CheckDiscType;
			}
			else if( coverState & 2 )// disc was present before isnt is gone now
			{
				instance->DiscEjected();
				//gprintf( "disc ejected\n" );
				state = St_WaitForDisc;
			}
			coverState = cover;
		}
		if( !( cover & 2 ) )// if theres no disc inserted, then loop
		{
			continue;
		}

		if( state == St_WaitForDiscEject )
		{
			continue;
		}
		else if( state == St_CheckDiscType )
		{
			s32 ret = WDVD_ReadDiskId( (void*)0x80000000 );
			if( ret < 0 )
			{
				gprintf("WDVD_ReadDiskId(): %d\n", ret );
				instance->ErrorHappened( E_DVD_ReadError, false );
				//state = St_WaitForDiscEject;
				WDVD_Close();
				state = St_Init;
				//coverState = 0;
				continue;
			}

			// check disc type
			if( *(u32*)( 0x80000018 ) == 0x5d1c9ea3 )
			{
				//gprintf( "disc is wii\n" );
				state = St_OpenPartition;
				//instance->DiscInserted( T_Wii );
			}
			else if( *(u32*)( 0x8000001c ) == 0xc2339f3d )
			{
				//gprintf( "disc is gamecube\n" );
				instance->DiscInserted( T_GC );
				state = St_Idle;
			}
			else
			{
				//gprintf( "disc is unknown\n" );
				instance->DiscInserted( T_Unknown );
				state = St_WaitForDiscEject;
				//hexdump( (void*)0x80000000, 0x20 );
			}
		}
		else if( state == St_OpenPartition )
		{
			if( WDVD_OpenDataPartition() < 0 )
			{
				instance->ErrorHappened( E_OpenPartition, false );
				state = St_WaitForDiscEject;
				continue;
			}
			//gprintf( "partition is open\n" );

			// search for the opening.bnr
			s32 ret;
			FST_INFO fst_info __attribute(( aligned( 32 ) ));

			//find FST inside partition
			ret = WDVD_Read( (u8*)&fst_info, sizeof( FST_INFO ), 0x420LL );
			if( ret < 0 )
			{
				gprintf("WDVD_Read( fst_info ): %d\n", ret );
				instance->ErrorHappened( E_DVD_ReadError, false );
				state = St_WaitForDiscEject;
				WDVD_ClosePartition();
				continue;
			}
			fst_info.fst_offset <<= 2;
			fst_info.fst_size <<= 2;
			//gprintf( "%s %i\n", __FILE__, __LINE__ );

			fst_buffer = (u8*)memalign( 32, RU( fst_info.fst_size, 0x40 ) );
			if( !fst_buffer )
			{
				instance->ErrorHappened( E_NoMem, true );
				threadExit = true;
				WDVD_ClosePartition();
				continue;
			}
			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			//gprintf( " %p  %08x %08x\n", fst_buffer, fst_info.fst_size, fst_info.fst_offset );

			//read fst into memory
			ret = WDVD_Read( fst_buffer, fst_info.fst_size, fst_info.fst_offset );
			if( ret < 0 )
			{
				gprintf("WDVD_Read( fst_buffer ): %d\n", ret );
				instance->ErrorHappened( E_DVD_ReadError, false );
				state = St_WaitForDiscEject;
				WDVD_ClosePartition();
				continue;
			}
			//gprintf( "%s %i\n", __FILE__, __LINE__ );

			//set the pointers
			fst = (FST_ENTRY *)fst_buffer;
			u32 name_table_offset = fst->filelen * 0xC;
			name_table = (char *)( fst_buffer + name_table_offset );

			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			// find the opening.bnr
			int fd = EntryFromPath( "/opening.bnr", 0 );
			if( fd < 2 )
			{
				instance->ErrorHappened( E_NoOpeningBnr, false );
				instance->DiscInserted( T_Wii );
				FREE( fst );
				name_table = NULL;
				state = St_Idle;
				WDVD_ClosePartition();
				continue;
			}
			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			u32 len = fst[ fd ].filelen;
			u8 *buf = (u8*)memalign( 32, RU( len, 0x40 ) );
			if( !buf )
			{
				instance->ErrorHappened( E_NoMem, true );
				threadExit = true;
				FREE( fst );
				name_table = NULL;
				WDVD_ClosePartition();
				continue;
			}
			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			ret = WDVD_Read( buf, len, (u64)( fst[ fd ].fileoffset ) << 2 );
			if( ret < 0 )
			{
				gprintf("WDVD_Read( opening.bnr ): %d\n", ret );
				instance->ErrorHappened( E_DVD_ReadError, false );
				state = St_WaitForDiscEject;
				FREE( fst );
				name_table = NULL;
				free( buf );
				WDVD_ClosePartition();
				continue;
			}

			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			// done with these
			FREE( fst );
			name_table = NULL;
			WDVD_ClosePartition();

			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			bool rec = false;
			// got the opening.bnr.  send it to whoever cares
			instance->OpeningBnrReady( buf, len, rec );
			if( !rec )
			{
				gprintf( "nobody got the banner.  freeing it\n" );
				free( buf );
			}
			instance->DiscInserted( T_Wii );
			//gprintf( "%s %i\n", __FILE__, __LINE__ );
			state = St_Idle;
		}
	}

	return NULL;
}
