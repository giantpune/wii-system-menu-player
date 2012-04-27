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
#include "systemmenuresource.h"
#include "SystemFont.h"

SystemMenuResource::SystemMenuResource()
	: loaded( false )
{
}

Layout *SystemMenuResource::LoadLayout( const U8Archive &arc, const std::string &lytName )
{
	// read layout data
	u8 *stuff = arc.GetFile( "/arc/blyt/" + lytName + ".brlyt" );
	if( !stuff )
	{
		return NULL;
	}

	// load layout
	Layout *ret = new Layout;
	if( !ret->Load( stuff ) )
	{
		delete ret;
		return NULL;
	}
	//ret->SetLanguage( "ENG" );

	// load fonts and textures
	//ret->LoadFonts( U8Archive(SystemFont::GetFont(), SystemFont::GetFontSize()) );
	ret->LoadFonts( arc );
	if( !ret->LoadTextures( arc ) )
	{
		delete ret;
		return NULL;
	}

	//gprintf( "loaded layout: \"%s\" %.2f x %.2f\n", lytName.c_str(), ret->GetWidth(), ret->GetHeight() );
	return ret;
}

void SystemMenuResource::SetMaterialIndex( Layout *l, const char* name, u8 whichTextureMap, u8 idx )
{
	Material *mat = l->FindMaterial( name );
	if( !mat )
	{
		gprintf( "didnt find material: %s\n", name );
		return;
	}
	mat->SetTextureIndex( whichTextureMap, idx );
}

Animation *SystemMenuResource::LoadAnimation( const U8Archive &arc, const std::string &lanName )
{
	u8 *stuff = arc.GetFile( "/arc/anim/" + lanName + ".brlan" );
	if( !stuff )
	{
		return NULL;
	}
	Animation *brlan = new Animation( lanName );
	brlan->Load( (const RLAN_Header*)stuff );
	return brlan;
}

bool SystemMenuResource::SetText( Layout *layout, const char* textboxName, const char16 *text, bool failSilently )
{
	Textbox *tbox;
	if( (tbox = layout->FindTextbox( textboxName ) ) )
	{
		tbox->SetText( text );
		return true;
	}
	if( !failSilently )
	{
		gprintf( "textbox \"%s\" not found\n", textboxName );
	}
	return false;
}

void SystemMenuResource::SetPaneVisible( Layout *layout, const char* paneName, bool visible )
{
	Pane *pane;
	if( (pane = layout->FindPane( paneName ) ) )
	{
		pane->SetVisible( visible );
	}
	else
	{
		gprintf( "SystemMenuResource::SetPaneVisible(): \"%s\" not found\n", paneName );
	}
}
