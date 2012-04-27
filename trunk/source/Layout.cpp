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
#include "Layout.h"
#include "list.h"
#include "WiiFont.h"

Layout::Layout()
	: header(NULL)
{
}

Layout::~Layout()
{
	for(u32 i = 0; i < panes.size(); ++i)
		delete panes[i];

	for(u32 i = 0; i < resources.materials.size(); ++i)
		delete resources.materials[i];

	for(u32 i = 0; i < resources.textures.size(); ++i)
		delete resources.textures[i];

	for(u32 i = 0; i < resources.fonts.size(); ++i)
	{
		if( !resources.fonts[i]->isSystemFont )
		{
			delete resources.fonts[i];
		}
	}

}

bool Layout::Load(const u8 *brlyt)
{
	if(!brlyt)
		return false;

	const BRLYT_Header *brlytFile = (const BRLYT_Header *) brlyt;

	if(brlytFile->magic != BRLYT_MAGIC || brlytFile->version != BRLYT_VERSION)
		return false;

	Group* last_group = NULL;
	std::stack<std::map<std::string, Group>*> group_stack;
	group_stack.push(&groups);

	Pane* last_pane = NULL;
	std::stack<std::vector<Pane*>*> pane_stack;
	pane_stack.push(&panes);

	const u8 *position = brlyt + brlytFile->header_len;

	for(u32 i = 0; i < brlytFile->section_count; ++i)
	{
		section_t *section = (section_t *) position;
		position += section->size;

		if(section->magic == Layout::MAGIC)
		{
			header = (Layout::Header *) (section + 1);
		}
		else if (section->magic == TextureList::MAGIC)
		{
			const LytItemList *txl1 = (const LytItemList *) (section+1);
			const char *nameoffset = ((const char *)(txl1+1));
			const LytStringTable *stringTable = (const LytStringTable *) (((const u8 *)(txl1+1))+txl1->offset_to_first);

			for(u32 i = 0; i < txl1->num_items; ++i)
			{
				Texture *texture = new Texture;
				texture->setName(nameoffset+stringTable[i].offset_filename);
				resources.textures.push_back(texture);
			}
		}
		else if (section->magic == MaterialList::MAGIC)
		{
			const LytItemList *mat1 = (const LytItemList *) (section+1);
			const u32 *mat_offsets = (const u32 *) (((const u8 *)(mat1+1))+mat1->offset_to_first);

			for(u32 i = 0; i < mat1->num_items; ++i)
			{
				Material *material = new Material;
				material->Load((Material::Header *) (((const u8 *) section)+mat_offsets[i]));
				resources.materials.push_back(material);
			}
		}
		else if (section->magic == FontList::MAGIC)
		{
			// load font list
			const LytItemList *fnl1 = (const LytItemList *) (section+1);
			const char *nameoffset = ((const char *)(fnl1+1));
			const LytStringTable *stringTable = (const LytStringTable *) (((const u8 *)(fnl1+1))+fnl1->offset_to_first);

			for(u32 i = 0; i < fnl1->num_items; i++)
			{
				const char *name = nameoffset+stringTable[i].offset_filename;

				WiiFont *font = WiiFont::GetSystemFont( name );
				if( !font )
				{
					font = new WiiFont;
					font->SetName( name );
				}
				resources.fonts.push_back( font );
			}
		}
		else if (section->magic == Pane::MAGIC)
		{
			Pane* pane = new Pane;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Bounding::MAGIC)
		{
			Bounding* pane = new Bounding;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Picture::MAGIC)
		{
			Picture* pane = new Picture;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Window::MAGIC)
		{
			Window* pane = new Window;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Textbox::MAGIC)
		{
			Textbox* pane = new Textbox;
			pane->Load((Pane::Header *) section);
			pane_stack.top()->push_back(last_pane = pane);
		}
		else if (section->magic == Layout::MAGIC_PANE_PUSH)
		{
			if (last_pane)
				pane_stack.push(&last_pane->panes);
		}
		else if (section->magic == Layout::MAGIC_PANE_POP)
		{
			if (pane_stack.size() > 1)
				pane_stack.pop();
		}
		else if (section->magic == Group::MAGIC)
		{
			const char *grp = (const char *) (section + 1);
			std::string group_name(grp, 0, Layout::Group::NAME_LENGTH);
			Group& group_ref = (*group_stack.top())[group_name];
			grp += Layout::Group::NAME_LENGTH;

			u16 sub_count = *(u16 *) grp;
			grp += 4; // 2 bytes reserved

			while (sub_count--)
			{
				std::string pane_name(grp, 0, Layout::Group::NAME_LENGTH);

				Pane *thePane = FindPane( pane_name );
				if( thePane )
				{
					group_ref.panes.push_back( thePane );
				}
				else
				{
					gprintf( "didn\'t find pane: \"%s\" for group\n", pane_name.c_str() );
				}

				grp += Layout::Group::NAME_LENGTH;
			}

			last_group = &group_ref;
		}
		else if (section->magic == Layout::MAGIC_GROUP_PUSH)
		{
			if (last_group)
				group_stack.push(&last_group->groups);
		}
		else if (section->magic == Layout::MAGIC_GROUP_POP)
		{
			if (group_stack.size() > 1)
				group_stack.pop();
		}
		else {
			gprintf("Uknown layout section: %08X\n", section->magic);
		}
	}
	return true;
}

bool Layout::LoadTextures( const U8Archive &archive )
{
	bool success = true;

	for(u32 i = 0; i < resources.textures.size(); ++i)
	{
		const u8 *file = archive.GetFile( "/arc/timg/" + resources.textures[i]->getName() );
		if (file)
			resources.textures[i]->Load(file);
		else
			success = false;
	}

	return success;
}

bool Layout::LoadFonts( const U8Archive &archive )
{
	bool success = true;

	for(u32 i = 0; i < resources.fonts.size(); ++i)
	{
		if( resources.fonts[ i ]->IsLoaded() )
		{
			continue;
		}
		u32 fd = archive.FileDescriptor( "/arc/font/" + resources.fonts[i]->getName() );
		if( !fd )
		{
			gprintf( "error loading font: \"%s\"\n", resources.fonts[i]->getName().c_str() );
			continue;
		}
		const u8 *file = archive.GetFileFromFd( fd );
		if( file )
		{
			resources.fonts[i]->Load(file);
		}
		else
			success = false;
	}

	return success;
}

void Layout::RenderWithCurrentMtx( Mtx &modelview, bool widescreen ) const
{
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->Render(resources, 0xff, modelview, widescreen);
}

void Layout::Render(Mtx &modelview, const Vec2f &ScreenProps, bool widescreen, u8 render_alpha) const
{
	if(!header)
		return;

	Mtx mv;
	// we draw inverse
	guMtxScaleApply(modelview, mv, 1.0f, -1.0f, 1.0f);

	// centered draw
	if(header->centered)
		guMtxTransApply(mv, mv, ScreenProps.x * 0.5f, ScreenProps.y * 0.5f, 0.f);


	//guMtxTransApply(mv, mv, 0.f, 20.f, 0.f);

	// render all panes
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->Render(resources, render_alpha, mv, widescreen);
}

void Layout::SetLanguage(  std::string language )
{
	//gprintf( "SetLanguage( \"%s\" )\n", language.c_str() );
	// make sure the requested language is valid
	std::string eng( "ENG" );
	StringList langs;
	langs <<  "JPN" << eng << "GER" << "FRA" << "SPA" << "ITA" << "NED" << "CHN" << "KOR";
	if( !langs.Contains( language ) )
	{
		gprintf( "!langs.Contains( language )\n" );
		return;
	}

	const Group *root = FindGroup( "RootGroup" );
	if( !root )
	{
		gprintf( "!root\n" );
		return;
	}

	// check first to see if we have the requested language and/or english
	std::map<std::string, Group>::const_iterator it = root->groups.begin(), itE = root->groups.end();
	bool haveLanguage = false;
	bool haveEnglish = false;
	while( it != itE )
	{
		if( it->first == language )
		{
			haveLanguage = true;
		}
		if( it->first == eng )
		{
			haveEnglish = true;
		}
		++it;
	}

	if( !haveLanguage )
	{
		if( !haveEnglish )
		{
			// nothing to do
			return;
		}
		language = eng;
	}
	//gprintf( " using lang: \"%s\"\n", language.c_str() );

	// now go back through and show/hide panes
	it = root->groups.begin(), itE = root->groups.end();
	while( it != itE )
	{
		bool hide = ( it->first != language );
		if( hide )
		{
			//gprintf( "  SetGroup: \"%s\" (%u)  %u\n", it->first.c_str(), it->second.panes.size(), hide );
			foreach( Pane *pane, it->second.panes )
			{
				//gprintf( "   SetPane: \"%s\" %u\n", pane->getName(), hide );
				pane->SetHide( true );
				pane->SetVisible( false );
			}
		}
		++it;
	}

	it = root->groups.begin(), itE = root->groups.end();
	while( it != itE )
	{
		bool hide = ( it->first != language );
		if( !hide )
		{
			//gprintf( "  SetGroup: \"%s\" %u\n", it->first.c_str(), hide );
			foreach( Pane *pane, it->second.panes )
			{
				//gprintf( "   SetPane: \"%s\" %u\n", pane->getName(), hide );
				pane->SetHide( false );
				pane->SetVisible( true );
			}
		}
		++it;
	}
}

Pane* Layout::FindPane(const std::string& find_name)
{
	for(u32 i = 0; i < panes.size(); ++i)
	{
		Pane* found = panes[i]->FindPane(find_name);
		if(found)
			return found;
	}

	return NULL;
}

Pane* Layout::FindPane(const char* find_name)
{
	for(u32 i = 0; i < panes.size(); ++i)
	{
		Pane* found = panes[i]->FindPane(find_name);
		if(found)
			return found;
	}

	return NULL;
}

Textbox* Layout::FindTextbox( const char* name )
{
	Pane *pane = FindPane( name );
	if( !pane )
	{
		return NULL;
	}
	return static_cast< Textbox * >( pane );
}

Material* Layout::FindMaterial(const std::string& find_name)
{
	for(u32 i = 0; i < resources.materials.size(); ++i)
	{
		if (find_name.compare(0, 20, resources.materials[i]->getName()) == 0)
			return resources.materials[i];
	}

	return NULL;
}

Material* Layout::FindMaterial(const char* find_name)
{
	for(u32 i = 0; i < resources.materials.size(); ++i)
	{
		if( !strncmp( find_name, resources.materials[i]->getName(), 20 ) )
			return resources.materials[i];
	}

	return NULL;
}

Texture* Layout::FindTexture(const std::string& find_name)
{
	for(u32 i = 0; i < resources.textures.size(); ++i)
	{
		if (find_name == resources.textures[i]->getName())
			return resources.textures[i];
	}

	return NULL;
}

Texture* Layout::FindTexture(const char* find_name)
{
	for(u32 i = 0; i < resources.textures.size(); ++i)
	{
		if( !strcmp( find_name, resources.textures[i]->getName().c_str() ) )
			return resources.textures[i];
	}

	return NULL;
}

void Layout::LoadBrlanTpls( Animation* anim, const U8Archive &archive )
{
	if( !anim )
	{
		return;
	}
	// load textures for palette animations
	u32 size = anim->paletteNames.size();
	for( u32 i = 0; i < size; i++ )
	{
		const std::string &pName = anim->paletteNames[ i ];
		if( FindTexture( pName ) )
		{
			continue;
		}
		u32 filesize = 0;
		const u8 *file = archive.GetFile( "/arc/timg/" + pName, &filesize );
		if( !file )
		{
			gprintf( "failed to load texture from brlan: %s\n", pName.c_str() );
			continue;
		}
		Texture *texture = new Texture;
		texture->setName( pName );
		texture->Load( file );
		resources.textures.push_back( texture );
	}
}

const Layout::Group *Layout::FindGroup( const std::string &name ) const
{
	std::map<std::string, Group>::const_iterator it = groups.begin(), itE = groups.end();
	while( it != itE )
	{
		if( it->first == name )
		{
			return &it->second;
		}
		const Group *ret = FindGroup( name, &(it->second) );
		if( ret )
		{
			return ret;
		}
		++it;
	}
	gprintf( "Layout::FindGroup( \"%s\" ): not found\n", name.c_str() );
	return NULL;
}

const Layout::Group *Layout::FindGroup( const std::string &name, const Layout::Group *parent ) const
{
	std::map<std::string, Group>::const_iterator it = parent->groups.begin(), itE = parent->groups.end();
	while( it != itE )
	{
		if( it->first == name )
		{
			return &it->second;
		}
		const Group *ret = FindGroup( name, &(it->second) );
		if( ret )
		{
			return ret;
		}
		++it;
	}
	return NULL;
}
