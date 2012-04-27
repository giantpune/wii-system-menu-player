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
#ifndef WII_CURSORS_H
#define WII_CURSORS_H

#include "Banner.h"
#include "systemmenuresource.h"

// this is a class to deal with the cursors
class WiiCursors : public SystemMenuResource
{
public:
	WiiCursors();
	~WiiCursors();

	bool Load( const u8* chanTtlAshData, u32 chanTtlAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

private:
    static bool bInitialized;
    static Mtx44 projection;

	Layout *cursor[4];

	// panes for the cursors.  each one has 3
	//! translate
	Pane *transPane[4];

	//! rotate
	Pane *rotPane[4];

	//! rotate for the shadow
	Pane *rotSPane[4];
};

#endif // Cursor_H
