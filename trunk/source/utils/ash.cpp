#include "ash.h"
#include "gecko.h"


#include <gccore.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>



bool IsAshCompressed( const u8 *stuff, u32 len )
{
	return ( len > 0x10 &&
			 ((*(u32*)( stuff )) & 0xFFFFFF00 ) == 0x41534800 );
}

u8*	DecompressAsh( const u8 *stuff, u32 &len )
{
	if( !IsAshCompressed( stuff, len ) )
	{
		return NULL;
	}

	u32 r[32];
	u32 t;

	r[4] = (u32)stuff;      //in


	r[5] = *(u32*)( r[4] + 4 );
	r[5] = r[5] & 0x00FFFFFF;

	u32 size = r[5];

	//gprintf("Decompressed size: %d\n", size);
	u8* buf1 = (u8*)memalign( 32, size );
	if( !buf1 )
	{
		gprintf( "ASH: no memory\n" );
		return NULL;
	}
	r[3] = (u32)buf1;   //out
	//memset( (void*)buf1, 0, size );
	//printf("r[3] :%08X\n", r[3]);

	//printf("\n\n");

	r[24] = 0x10;
	r[28] = *(u32 *)(r[4]+8);
	r[25] = 0;
	r[29] = 0;
	r[26] = *(u32 *)(r[4]+0xC);
	r[30] = *(u32 *)(r[4]+r[28]);
	r[28] = r[28] + 4;
	//r[8]  = 0x8108<<16;
	//HACK, pointer to RAM
	u8* workingBuffer = (u8*)memalign( 32, MAX( 0x100000, size ) );
	if( !workingBuffer )
	{
		gprintf( "ASH: no memory 2\n" );
		free( buf1 );
		return NULL;
	}
	r[8]  = (u32)workingBuffer;
	//memset( (void*)workingBuffer, 0, 0x100000 );

	r[9]  = r[8]  + 0x07FE;
	r[10] = r[9]  + 0x07FE;
	r[11] = r[10] + 0x1FFE;
	r[31] = r[11] + 0x1FFE;
	r[23] = 0x200;
	r[22] = 0x200;
	r[27] = 0;

	while( 1 )
	{
		// loc_81332124:
		r[0] = r[26] >> 31;
		if( r[25] != 0x1F )
		{
			// loc_81332140:
			r[25]++;
			r[26] <<= 1;
		}
		else
		{
			// 8133212C
			r[26]= *(u32 *)( r[4] + r[24] );
			r[25]= 0;
			r[24] += 4;
		}

		// loc_8133214C:
		if( r[0] )
		{
			// 81332150
			*(u16*)( r[31] )	 = r[23] | 0x8000;
			*(u16*)( r[31] + 2 ) = r[23] | 0x4000;

			r[31] += 4;
			r[27] += 2;
			r[23]++;
			r[22]++;
			continue;
		}

		// loc_81332174:
		r[21] = r[25] + 9;
		t = r[21];
		if( r[21] > 0x20 )
		{
			// loc_813321AC:
			r[0] = 0x17;
			r[6] = r[26] >> r[0];
			r[26]= *(u32*)( r[4] + r[24 ]);
			r[0] = (~(r[21] - 0x40))+1;
			r[24] += 4;
			r[0] = r[26] >> r[0];
			r[6] |= r[0];
			r[25] = r[21] - 0x20;
			r[26] = r[26] << r[25];
		}
		else
		{
			// 81332184
			r[21] = 0x17;
			r[6]  = r[26] >> 0x17;
			if( t == 0x20 )
			{
				// loc_8133219C:
				r[26]= *(u32 *)(r[4] + r[24]);
				r[25]= 0;
				r[24] += 4;
			}
			else
			{
				// 81332190
				r[26] <<= 9;
				r[25] += 9;
			}
		}

		// loc_813321D0:
		bool continueLoop = false;
		while( 1 )
		{
			r[12]= *(u16*)( r[31] - 2 );
			r[31] -= 2;
			r[27]--;
			r[0] = r[12] & 0x8000;
			r[12]= (r[12] & 0x1FFF) << 1;
			if( !r[0] )
			{
				*(u16*)( r[8]+r[12] ) = r[6];
				r[23] = r[22];
				continueLoop = true;
				break;
			}
			else
			{
				*(u16*)(r[9]+r[12]) = r[6];
				r[6] = (r[12] & 0x3FFF)>>1;                     //   extrwi  %r6, %r12, 14,17
				if( r[27] != 0 )
				{
					continue;
				}
				break;
			}
		}

		if( !continueLoop )
		{
			break;
		}
	}

	// loc_81332204:
	r[23] = 0x800;
	r[22] = 0x800;

	// loc_8133220C:
	while( 1 )
	{
		if( r[29] != 0x1F )
		{
			// loc_81332228:
			r[0] = r[30] >> 31;
			r[29]++;
			r[30] <<= 1;
		}
		else
		{
			// 81332214
			r[0] = r[30] >> 31;
			r[30] = *(u32*)(r[4] + r[28]);
			r[29] = 0;
			r[28] += 4;
		}

		if( r[0] )
		{
			r[0] = r[23] | 0x8000;
			*(u16 *)(r[31]) = s16(r[0]);
			r[0] = r[23] | 0x4000;
			*(u16 *)(r[31]+2) = s16(r[0]);

			r[31] += 4;
			r[27] += 2;
			r[23]++;
			r[22]++;

			continue;
		}
		else
		{
			//loc_8133225C:
			r[21] = r[29] + 0xb;
			t = r[21];
			if( r[21] > 0x20 )
			{
				//loc_81332294:
				r[7] = r[30] >> 0x15;
				r[30]= *(u32 *)(r[4] + r[28]);
				r[0] = (~(r[21] - 0x40))+1;
				r[28] += 4;
				r[0] = r[30] >> r[0];
				r[7] |= r[0];
				r[29]= r[21] - 0x20;
				r[30]= r[30] << r[29];
			}
			else
			{
				// 8133226C
				r[7]  = r[30] >> 0x15;
				if( t == 0x20 )
				{
					// loc_81332284:
					r[30]= *(u32 *)(r[4] + r[28]);
					r[29]= 0;
					r[28] += 4;
				}
				else
				{
					// 81332278
					r[30] <<= 0xb;
					r[29] += 0xb;
				}
			}
		}

		// 813322B8:
		bool loop = true;
		while( 1 )
		{
			r[12]= *(u16*)(r[31] - 2);
			r[31] -= 2;
			r[27]= r[27] - 1;
			r[0] = r[12] & 0x8000;
			r[12]= (r[12] & 0x1FFF) << 1;
			if( r[0] == 0 )
			{
				// 813322E0;
				*(u16 *)(r[10]+r[12]) = s16(r[7]);
				r[23] = r[22];
				break;
			}
			else
			{
				// 813322CC
				*(u16 *)(r[11]+r[12]) = s16(r[7]);
				r[7] = (r[12] & 0x3FFF)>>1;                     // extrwi  %r7, %r12, 14,17
				if( !r[27] )
				{
					loop = false;
					break;
				}
			}
		}
		if( !loop )
		{
			break;
		}
	}



	// 813322EC:
	r[0] = r[5];

loc_813322F0:

	r[12]= r[6];

loc_813322F4:

	if( r[12] < 0x200 )
		goto loc_8133233C;

	if( r[25] != 0x1F )
		goto loc_81332318;

	r[31] = r[26] >> 31;
	r[26] = s32(*(u32 *)(r[4] + r[24]));
	r[24] = r[24] + 4;
	r[25] = 0;
	goto loc_81332324;

loc_81332318:

	r[31] = r[26] >> 31;
	r[25] = r[25] +  1;
	r[26] = r[26] << 1;

loc_81332324:

	r[27] = r[12] << 1;
	if( r[31] != 0 )
		goto loc_81332334;

	r[12] = *(u16*)(r[8] + r[27]);
	goto loc_813322F4;

loc_81332334:

	r[12] = *(u16*)(r[9] + r[27]);
	goto loc_813322F4;

loc_8133233C:

	if( r[12] >= 0x100 )
		goto loc_8133235C;

	*(unsigned char *)(r[3]) = r[12];
	r[3] = r[3] + 1;
	r[5] = r[5] - 1;
	if( r[5] != 0 )
		goto loc_813322F0;

	goto loc_81332434;

loc_8133235C:

	r[23] = r[7];

loc_81332360:

	if( r[23] < 0x800 )
		goto loc_813323A8;

	if( r[29] != 0x1F )
		goto loc_81332384;

	r[31] = r[30] >> 31;
	r[30] = s32(*(u32 *)(r[4] + r[28]));
	r[28] = r[28] + 4;
	r[29] = 0;
	goto loc_81332390;

loc_81332384:

	r[31] = r[30] >> 31;
	r[29] = r[29] +  1;
	r[30] = r[30] << 1;

loc_81332390:

	r[27] = r[23] << 1;
	if( r[31] != 0 )
		goto loc_813323A0;

	r[23] = *(u16*)(r[10] + r[27]);
	goto loc_81332360;

loc_813323A0:

	r[23] = *(u16*)(r[11] + r[27]);
	goto loc_81332360;

loc_813323A8:

	r[12] -= 0xFD;
	r[23] = ~r[23] + r[3] + 1;
	r[5]  = ~r[12] + r[5] + 1;
	r[31] = r[12] >> 3;

	// loc_813323C0:
	for( u32 i = 0; i < r[31]; i++, r[23] += 8, r[3] += 8 )
	{
		*(u8*)(r[3])	= *(u8*)(r[23] - 1);
		*(u8*)(r[3]+1)	= *(u8*)(r[23]);
		*(u8*)(r[3]+2)	= *(u8*)(r[23] + 1);
		*(u8*)(r[3]+3)	= *(u8*)(r[23] + 2);
		*(u8*)(r[3]+4)	= *(u8*)(r[23] + 3);
		*(u8*)(r[3]+5)	= *(u8*)(r[23] + 4);
		*(u8*)(r[3]+6)	= *(u8*)(r[23] + 5);
		*(u8*)(r[3]+7)	= *(u8*)(r[23] + 6);
	}

	r[12] &= 7;

	// 81332414:
	for( u32 i = 0; i < r[12]; i++ )
	{
		r[31] = *(u8*)(r[23] - 1);
		r[23]++;
		*(u8*)(r[3]) = r[31];
		r[3]++;
	}

	// 8133242C:
	if( r[5] != 0 )
		goto loc_813322F0;

loc_81332434:
	len = r[0];

	//gprintf("Decompressed %d bytes\n", r[3]);
	free( workingBuffer );
	return buf1;
}
