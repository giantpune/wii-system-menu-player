#ifndef ASH_H
#define ASH_H

#include <gctypes.h>

// check if data is ash compressed
bool IsAshCompressed( const u8 *stuff, u32 len );

// decompress ash compressed data
//! len is the size of the compressed data, and is set to the size of the decompressed data
//! this allocates memory with memalign, free it when you are done with it
u8*	DecompressAsh( const u8 *stuff, u32 &len );

#endif // ASH_H
