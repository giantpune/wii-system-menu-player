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
#ifndef GCBANNER_H
#define GCBANNER_H

#include "object.h"
#include "systemmenuresource.h"

class GCBanner : public SystemMenuResource
{
public:
	GCBanner();
	~GCBanner();

	bool Load( const u8* GCBannAshData, u32 GCBannAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

protected:
	Layout *lyt;
	Object *obj;
	Animation *lan;// theres actually 4 brlans, but we only care to use 1 of them
};


#endif // GCBANNER_H
