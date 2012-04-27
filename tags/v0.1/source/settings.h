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
#ifndef SETTINGS_H
#define SETTINGS_H

#include "tinyxml/tinyxml.h"

namespace Settings
{

// read banners (.bnr / .app ) from sd card
extern bool useDumpedBanners;

// show banners of channels on the nand
extern bool useNandBanners;

// load sd:/app/... homebrew and create banners
extern bool useHomebrewForBanners;

// try to load the banner from DVD for the disc channel
extern bool mountDVD;

// path to read banners from on SD card.  default is SD:/banners/
extern std::string sdBannerPath;


// save and load
void Load( int argc, char *argv[] );
}

#endif // SETTINGS_H
