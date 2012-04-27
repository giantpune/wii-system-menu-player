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

#ifndef _BANNER_H_
#define _BANNER_H_

#include "Layout.h"
#include "SystemMenu/object.h"
#include "utils/U8Archive.h"

class Banner
{
public:
	Banner( const u8* data, u32 len );
	~Banner();

	bool Load( const u8 *data, u32 len );

	virtual Object *LoadBanner();
	virtual Object *LoadIcon();
	bool LoadSound();

	// delete the banner and icon in case you wanna save memory or something stupid like that
	void UnloadBanner();
	void UnloadIcon();
	void UnloadSound();

	Layout *getBanner() const { return layout_banner; }
	Layout *getIcon() const { return layout_icon; }
	const u8 *getSound() const { return sound_bin; }
	u32 getSoundSize() const { return sound_bin_size; }

	const char16 *GetTitle() const;
	const char16 *GetSubTitle() const;

protected:
	U8Archive *arc;
	Object *bannerObj;
	Object *iconObj;

	std::map< std::string, Animation *>iconBrlans;
	std::map< std::string, Animation *>bannerBrlans;

	//Layout* LoadLayout(const u8 *banner_file, const std::string& lyt_name, const std::string &language);

	Layout *LoadLayout( const U8Archive &theArc, const std::string &lytName );
	Animation *LoadAnimation( const U8Archive &theArc, const std::string &lanName );

	u8 *banner_bin, *icon_bin, *sound_bin;
	u32 sound_bin_size;
	Layout *layout_banner, *layout_icon;

	u8* imetHeader;

	// stupid rso# groups
};

#endif
