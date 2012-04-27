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
#ifndef SYSTEMMENURESOURCE_H
#define SYSTEMMENURESOURCE_H

#include <map>

#include "bmg.h"
#include "Layout.h"
#include "utils/list.h"
#include "utils/sigslot.h"
#include "U8Archive.h"

// i guess this will be a common base class for different resources
class SystemMenuResource : public sigslot::has_slots<>
{
public:
	SystemMenuResource();
	virtual ~SystemMenuResource() {}
	// draw
	virtual void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen ){}


protected:

	bool loaded;

	// expects only the basename for the brlyt and brlan files from the archive
	//! tries to load the layout and animations
	static Layout *LoadLayout( const U8Archive &arc, const std::string &lytName );
	static Animation *LoadAnimation( const U8Archive &arc, const std::string &lanName );

	static void SetMaterialIndex( Layout *l, const char* name, u8 whichTextureMap, u8 idx );
	static void SetMaterialIndex( Layout *l, const std::string &name, u8 whichTextureMap, u8 idx )
	{
		return SetMaterialIndex( l, name.c_str(), whichTextureMap, idx );
	}

	static bool SetText( Layout *layout, const char* textboxName, const char16 *text, bool failSilently = false );
	static inline bool SetText( Layout *layout, const char* textboxName, int bmgIdx, bool failSilently = false )
	{
		return SetText( layout, textboxName, Bmg::Instance()->GetMessage( bmgIdx ), failSilently );
	}

	static void SetPaneVisible( Layout *layout, const char* paneName, bool visible );
};

#endif // SYSTEMMENURESOURCE_H
