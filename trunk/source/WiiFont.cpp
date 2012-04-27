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
#include "WiiFont.h"

WiiFont::WiiFont()
	: isSystemFont( false ),
	  header(NULL)
	, finf(NULL)
	, tglp(NULL)
	, cwdh(NULL)
	, font_loaded(false)
{
}

WiiFont::~WiiFont()
{
	std::map<u16, TextureCache>::iterator itr;

	for (itr = textureMap.begin(); itr != textureMap.end(); ++itr)
	{
		if(itr->second.allocated && itr->second.texture_data)
			free(itr->second.texture_data);
	}
}

bool WiiFont::Load(const u8 *file)
{
	header = (WiiFont::Header *) file;

	if((header->magic != MAGIC_FONT && header->magic != MAGIC_FONT_ARCHIVE)
		|| header->version != MAGIC_VERSION)
		return false;

	const u8 *position = ((const u8 *)header) + header->header_len;

	for(u32 i = 0; i < header->section_count; ++i)
	{
		section_t *section = (section_t *) position;
		position += section->size;

		switch( section->magic )
		{
		case MAGIC_GLYPH_GROUP:
			glgr = (GlgrHeader *) section;
			break;
		case MAGIC_FONT_INFORMATION:
			finf = (FinfHeader *) section;
			break;
		case MAGIC_TEXTURE_GLYPH:
			tglp = (TglpHeader *) section;
			break;
		case MAGIC_CHARACTER_WIDTH:
			cwdh = (CwdhHeader *) section;
			break;
		case MAGIC_CHARACTER_CODE_MAP:
			ParseCmap((CmapEntry *) (section + 1));
			break;
		default:
			// ignore
			gprintf("Uknown section %.4s\n", (char *) &section->magic);
			break;
		}
	}

	// Some sanity checks
	if(!finf || !tglp || !cwdh)
		return false;

	if(finf->tglpOffset > header->filesize || finf->cwdhOffset > header->filesize
		|| finf->cmapOffset > header->filesize)
		return false;

	font_loaded = true;

	return true;
}

inline bool WiiFont::CheckCmap(u16 charCode, u16 mapValue)
{
	std::map<u16, u16>::iterator it = cmap.find(charCode);
	if(it != cmap.end())
	{
		if((*it).second != mapValue)
		{
			gprintf("Duplicate characters\n");
			return false;
		}
	}
	return true;
}

bool WiiFont::ParseCmap(CmapEntry *cmapEntry)
{
	if(!cmapEntry)
		return false;

	while(true)
	{
		switch(cmapEntry->type)
		{
			case 0:
			{
				for(u16 i = cmapEntry->charCode, j = cmapEntry->start; j < cmapEntry->end; j++, i++)
				{
					if(!CheckCmap(j, i))
						return false;
					cmap[j] = i;
				}
				break;
			}
			case 1:
			{
				u16 idx = 0;
				u16 *idxPointer = &cmapEntry->charCode;
				for(u32 i = cmapEntry->start; i < cmapEntry->end; i++)
				{
					u16 m_idx = idxPointer[idx++];
					if(m_idx == 0xffff)
						continue;

					if(!CheckCmap(i, m_idx))
						return false;

					cmap[i] = m_idx;
				}
				break;
			}
			case 2:
			{
				u16 ind, character;
				u16 *charData = (u16 *) (cmapEntry + 1);
				for(u32 i = 0; i < cmapEntry->charCode; i++)
				{
					character = charData[0];
					ind = charData[1];
					charData += 2;

					if(!CheckCmap(character, ind))
						return false;

					cmap[character] = ind;
				}
				break;
			}
			default:
				gprintf( "unknown cmap type\n" );
				return false;
		}

		if(cmapEntry->pos == 0)
			return true;

		cmapEntry = (CmapEntry *)(((u8 *)header) + cmapEntry->pos);
	}
}

const WiiFont::CharInfo *WiiFont::GetCharInfo(u16 charCode)
{
	if(!finf || !tglp || !cwdh)
		return NULL;

	// see if the character already exists in our cache
	std::map<u16, CharInfo>::iterator itr = charInfoMap.find(charCode);
	if(itr != charInfoMap.end())
		return &itr->second;

	u16 idx = CharToIdx(charCode);
	if(idx > cwdh->endIdx)
	{
		gprintf( "idx > cwdh->endIdx" );
		return NULL;
	}

	Cwdh *cwdh2 = (Cwdh*) (((u8 *)header) + finf->cwdhOffset + 8);

	u32 chars_per_texture = (tglp->charColumns * tglp->charRows);
	u32 tex_idx = idx / chars_per_texture;
	u32 row = (idx - chars_per_texture * tex_idx) / tglp->charColumns;
	u32 col = (idx - chars_per_texture * tex_idx) - ( tglp->charColumns * row );
	f32 texRatioX = (float)(tglp->cellWidth + 1) / (float)tglp->width;
	f32 texRatioY = (float)(tglp->cellHeight + 1) / (float)tglp->height;

	f32 _s1 = texRatioX * (f32)col;
	f32 _s2 = _s1 + texRatioX;

	f32 _t1 = texRatioY * (f32)row;
	f32 _t2 = _t1 + texRatioY;

	CharInfo charInfo;
	charInfo.sheetIdx = tex_idx;
	charInfo.s1 = _s1;
	charInfo.s2 = _s2;
	charInfo.t1 = _t1;
	charInfo.t2 = _t2;
	charInfo.advanceGlyphX = cwdh2[idx].advanceGlyphX;
	charInfo.unk = cwdh2[idx].unk;
	charInfo.advanceKerning = cwdh2[idx].advanceKerning;

	charInfoMap[charCode] = charInfo;
	return &charInfoMap[charCode];
}

GXTexObj *WiiFont::LoadTextureObj(u16 tex_idx)
{
	if(!font_loaded || tex_idx >= tglp->texCnt )
		return NULL;

	// init texture and add it to the list
	TextureCache chacheStruct;

	if( header->magic == MAGIC_FONT_ARCHIVE )
	{
		chacheStruct.texture_data = GetUnpackedTexture(tex_idx);
		chacheStruct.allocated = true;

		if( !chacheStruct.texture_data )
			return NULL;
	}
	else
	{
		chacheStruct.texture_data = ((u8 *) header) + tglp->dataOffset + tex_idx * tglp->texSize;
		chacheStruct.allocated = false;
	}

	GX_InitTexObj( &chacheStruct.texObj, chacheStruct.texture_data, tglp->width, tglp->height, 0, 0, 0, true );
	GX_InitTexObjLOD( &chacheStruct.texObj, GX_LINEAR,GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_DISABLE, GX_FALSE, GX_ANISO_1 );

	textureMap[ tex_idx ] = chacheStruct;
	return &textureMap[ tex_idx ].texObj;
}

bool WiiFont::Apply(u16 tex_indx)
{
	if( !font_loaded || tex_indx >= tglp->texCnt )
		return false;

	GXTexObj *tex_obj;

	// Load character texture from cache if available otherwise create cache
	std::map<u16, TextureCache>::iterator itr = textureMap.find(tex_indx);
	if( itr != textureMap.end() )
	{
		tex_obj = &itr->second.texObj;
	}
	else if( !( tex_obj = LoadTextureObj( tex_indx ) ) )
	{
		// no texture found for this character
		return false;
	}

	GX_LoadTexObj( tex_obj, GX_TEXMAP0 );
	GX_InvalidateTexAll();
	return true;
}

// giantpunes little magic function
bool WiiFont::Decompress_0x28( unsigned char *outBuf, u32 outLen, const unsigned char *inBuf, u32 inLen )
{
	if( outLen & 3 )// this copies 32 bits at a time, so it probably needs to be aligned
	{
		gprintf( "length not aligned to 32 bits\n" );
		return false;
	}
	const u32 root_offset = 5;
	u32 symbol = 0;
	u32 counter = 0;
	u32 inIdx = 0;
	u32 outIdx = 0;
	u32 *in32 = (u32*)( ( inBuf + 6 ) + ( ( inBuf[ 4 ] << 1 ) ) );
	u32 *out32 = (u32*)outBuf;
	u8 *p = (u8*)inBuf + root_offset;

	outLen >>= 2; //we are copying 4 bytes at a time
	while( 1 )
	{
		for( u32 i = 0, leaf = p[ 0 ], bits = __builtin_bswap32( in32[ inIdx ] )
			 ; i < 0x20
			 ; i++, leaf = p[ 0 ] )
		{
			u32 bit = ( bits >> ( 31 - i ) ) & 1;
			u8 d = 2 - ((size_t)p & 1);
			p += bit + ( ( leaf << 1 ) & 0x7e ) + d;

			if( p > (u8*)inBuf + inLen )
			{
				gprintf( "out of range 1\n" );
				return false;
			}

			if( ( ( leaf << bit ) & ( 1 << 7 ) ) )					// p->isLeaf
			{
				symbol = ( ( p[ 0 ] << 0x18 ) | ( symbol >> 8 ) );
				p = (u8*)inBuf + root_offset;						// p = root
				if( counter++ > 2 )
				{
					out32[ outIdx++ ] = __builtin_bswap32( symbol );// buf[bufcur++] = p->symbol
					counter = 0;									// reset counter
					if( !--outLen )									// decrease amount to copy
					{
						return true;
					}
				}
			}
		}
		if( inIdx++ >= ( inLen >> 2 ) )
		{
			gprintf( "out of range 2\n" );
			return false;
		}
	}
	__builtin_unreachable();
}

u8 *WiiFont::GetUnpackedTexture(u16 sheetNo)
{
	u8 *data = (u8 *) header;
	u32 compressedSize;
	u32 off = 0;

	// skip over all the sheets till we get the one we want
	for( u32 i = 0; i < sheetNo; i++ )
	{
		compressedSize = *(u32*)( data + tglp->dataOffset + off );
		off += compressedSize + 4;
	}

	compressedSize = *(u32*)( data + tglp->dataOffset + off );

	u32 uncompressedSize = *(u32*)( data + tglp->dataOffset + off + 4 );
	if( (uncompressedSize & 0xff000000) != 0x28000000 )// looks like all the sheets in wbf1 and wbf2.brfna are 0x28
	{
		gprintf( "Brfna::LoadSheets(): unknown data type\n" );
		return NULL;
	}
	uncompressedSize = ( __builtin_bswap32( uncompressedSize ) >> 8 );
	if( !uncompressedSize )// is this right?  it looks like it is.  but it SHOULD only happen for files over 0xffffff bytes
	{
		uncompressedSize = __builtin_bswap32(*(u32*)( data + tglp->dataOffset + off + 8 ));
	}
	if( uncompressedSize != glgr->sheet_size )
	{
		gprintf( "uncompressedSize != glgr->sheet_size  %08x\n", uncompressedSize );
		return NULL;
	}

	// decompress
	u8* sheetData = (u8*)memalign( 32, uncompressedSize );// buffer needs to be 32bit aligned
	if( !sheetData )
		return NULL;

	if( !Decompress_0x28( sheetData, uncompressedSize, ( data + tglp->dataOffset + off + 4 ), compressedSize ) )
	{
		free( sheetData );
		return NULL;
	}

	// Flush cache because this is texture data getting fed straight to GX
	DCFlushRange( sheetData, uncompressedSize );

	return sheetData;
}

const char* WiiFont::ConvertToSystemFontName( const char* name )
{
	if( !strcmp( name, "wbf1_CN2.brfna" )
			|| !strcmp( name, "wbf1_KR2.brfna" )
			|| !strcmp( name, "RevoIpl_RodinNTLGPro_DB_32_I4.brfnt" )
			|| !strcmp( name, "RevoIpl_RodinNTLGPro_DB_48_IA4.brfnt" )
			|| !strcmp( name, "RevoIpl_UtrilloProGrecoStd_M_32_I4.brfnt" ) )
	{
		return "wbf1.brfna";
	}
	return name;
}

WiiFont *WiiFont::GetSystemFont( const char* name )
{
	name = ConvertToSystemFontName( name );
	if( !strcmp( name, "wbf1.brfna" ) )
	{
		return SystemFont::wbf1;
	}
	if( !strcmp( name, "wbf2.brfna" ) )
	{
		return SystemFont::wbf2;
	}
	return NULL;
}
