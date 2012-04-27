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
#ifndef DISCCHANNELICON_H
#define DISCCHANNELICON_H

#include "object.h"
#include "systemmenuresource.h"
#include "utils/char16.h"

// this is a class to handle the icon for the disc channel
class DiscChannelIcon : public SystemMenuResource
{
public:
	DiscChannelIcon();
	~DiscChannelIcon();

	bool Load( const u8* diskThumbAshData, u32 diskThumbAshSize );

	// dont call this.
	//void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	Layout *GetNoDiscLayout(){ return lyt1; }
	Object *GetNoDiscObject(){ return obj1; }

	Layout *GetInsertDiscLayout(){ return lyt2; }
	Object *GetInsertDiscObject(){ return obj2; }

	Layout *GetGCLayout(){ return lyt3; }
	Object *GetGCObject(){ return obj3; }

	void StartInsertDiscAnim();
	void StartEjectDiscAnim();

protected:
	Layout *lyt1;
	Layout *lyt2;
	Layout *lyt3;

	Object *obj1;
	Object *obj2;
	Object *obj3;

	std::map< std::string, Animation *>brlans;
};

#endif // DISCCHANNELICON_H
