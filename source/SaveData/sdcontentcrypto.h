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
#ifndef SDCONTENTCRYPTO_H
#define SDCONTENTCRYPTO_H

#include <gctypes.h>
#include <gccore.h>
#include <stdio.h>

// this is some misc crypto stuff used for content that has been moved to SD

// bk header used in saves and SD channels
//! some fields are used for one or the other, and some fields are used for both
struct BkHeader
{
	u32 _0x70;						// 0x00000070
	u32 magic;						// 0x426b0001
	u32 deviceID;					// id of the wii that created the file
	u32 numEntries;					// used for save data.bin

	u32 size1;						// data.bin
	u32 tmdSize;					// content.bin
	u32 payloadSize;				// content.bin
	u32 size2;						// file size from the start of the bk header to the end of the cert stuff

	u8 includedContents[ 0x40 ];	// content.bin.    which contents are included.  i bit per content

	u64 tid;						// data.bin
	u8 macAddy[ 6 ];				// data.bin
	u16 pad1;
}__attribute__(( packed ));

s32 ES_VerifySign( u8* sigHash,	u32 sigHashSize, u8* sig, u8* certs, u32 certLen );
s32 DecryptWithSDKey( u8* in, u32 len, u8* out );

// bkData is a pointer to the section in a data.bin or content.bin
//! it should be aligned to 32 bits since the data is passed to IOS directly from it
bool VerifyBKWad( u8* bkData, u32 len );

// certs that get populated by reading them from the system menu binary
extern u8 ca_dpki[ 0x400 ];
extern u8 ca_ppki[ 0x400 ];
extern u8 ms_dpki[ 0x240 ];
extern u8 ms_ppki[ 0x240 ];

#endif // SDCONTENTCRYPTO_H
