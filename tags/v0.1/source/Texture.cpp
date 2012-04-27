/*
Copyright (c) 2010 - Wii Banner Player Project
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

#include <set>
#include <string.h>
#include "Texture.h"
#include "video.h"

void Texture::Load(const u8 *file )
{
	header = (Texture::Header *) file;

	if (header->magic != MAGIC)
		return;	// bad header

	u32 texture_count = header->num_textures;
	// only support a single texture
	if (texture_count > 1)
	{
		// Never saw it happen
		texture_count = 1;
		gprintf("texture count > 1\n");
	}

	// read textures
	const TPL_Texture *tpl_list = (const TPL_Texture *) (file + header->header_size);

	for(u32 i = 0; i < texture_count; i++)
	{
		// seek to texture header
		const TPL_Texture_Header *texture = (const TPL_Texture_Header *) (file + tpl_list[i].texture_offset);

		u8 mipmap = 0;
		u8 bias_clamp = 0;

		if(texture->max_lod > 0)
			mipmap = GX_TRUE;
		if(texture->lod_bias > 0.0f)
			bias_clamp = GX_ENABLE;

		// texture data
		u8 *texture_data = (u8 *) (file + texture->offset);

		// seek to/read palette header
		if (tpl_list[i].palette_offset != 0)
		{
			palette = (TPL_Palette_Header *) (file + tpl_list[i].palette_offset);

			// load the texture
			GX_InitTexObjCI(&texobj, texture_data, texture->width, texture->height, texture->format,
							   texture->wrap_s, texture->wrap_t, mipmap, 0);
		}
		else
		{
			// load the texture
			GX_InitTexObj(&texobj, texture_data, texture->width, texture->height, texture->format,
								   texture->wrap_s, texture->wrap_t, mipmap);
		}

		// filter mode
		if(mipmap)
		{
			GX_InitTexObjLOD(&texobj, texture->min, texture->mag, texture->min_lod, texture->max_lod,
							 		  texture->lod_bias, bias_clamp, bias_clamp, texture->edge_lod);
		}
	}
	loaded = true;
}

void Texture::Apply(u8 &tlutName, u8 map_id, u8 wrap_s, u8 wrap_t) const
{
	if( !loaded )
	{
		gprintf( "Texture::Apply(): not loaded yet\n" );
		return;
	}

	if(tlutName >= 20 || map_id >= 8)
	{
		gprintf( "Texture::Apply(): bad parameters\n" );
		return;
	}

    // create a temporary texture object to not modify the original with the wrap_s and wrap_t parameters
	GXTexObj tempTexObj;
	for(int i = 0; i < 8; ++i)
		tempTexObj.val[i] = texobj.val[i];

	// assume that if there is a palette header, then this format is a CIx one
	if(palette)
	{
		// seek to/read palette data
		u8 *tlut_data = (u8 *) (((u8 *) header) + palette->offset);

		// load tlut
		GXTlutObj tlutobj;
		GX_InitTlutObj(&tlutobj, tlut_data, palette->format, palette->num_items );
		GX_LoadTlut(&tlutobj, tlutName);
		GX_InitTexObjTlut(&tempTexObj, tlutName);
		tlutName++;
	}

    GX_InitTexObjWrapMode(&tempTexObj, wrap_s, wrap_t);
	GX_LoadTexObj(&tempTexObj, map_id);
}

void Texture::LoadFromRawData( u8 *data, u16 width, u16 height, u8 fmt )
{
	if( !data )
	{
		return;
	}
	loaded = true;
	GX_InitTexObj( &texobj, data, width, height, fmt, 0, 0, false );
	//GX_InitTexObjLOD( &texObjs[ texIdx ], 0, 0, 0, 0,
	//						  0, 0, 0, 0 );
}
