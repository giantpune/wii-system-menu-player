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
#ifndef SYSTEM_FONT_H
#define SYSTEM_FONT_H

#include <gccore.h>
#include <ogcsys.h>
#include <map>

#include "WiiFont.h"

class WiiFont;
class SystemFont
{
public:
	// load the shared font archives from nand and parse them
	//! returns succees if both global fonts were initialized, but will still try its best to initialize
	//! one of them if it can
	static bool Init();

	// cleanup all the allocated memory for the shared wbf fonts
	static void DeInit();

	static const u8 *GetFont() { return fontArchive; }
	static u32 GetFontSize() { return archiveFilesize; }

	static WiiFont *wbf1;
	static WiiFont *wbf2;

private:

	// this is the u8 archive that holds the wbf1/wbf2 fonts
	static u8 *fontArchive;
	static u32 archiveFilesize;
};

#endif // BRFNT_H
