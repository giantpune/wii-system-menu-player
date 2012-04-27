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
#ifndef FATALERRORDIALOG_H
#define FATALERRORDIALOG_H

#include "systemmenuresource.h"

// this archive contains its own font.  of the usual ascii letters, it seems to be missing
// zGQXYZ, and most of the punctuation.  The only punctuation i see in it is
// -',.
// I haven't check which of the goofy foreign letters it suports
class FatalErrordialog : public SystemMenuResource
{
public:
	FatalErrordialog() : layout( NULL ), tbox( NULL )
	{
	}
	~FatalErrordialog()
	{
		delete layout;
	}

	bool Load( const u8* fatalDlgAshData, u32 fatalDlgAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	// set the error message
	void SetMessage( const char16* message ) { if( tbox ){tbox->SetText( message );} }
protected:
	Layout *layout;
	Textbox *tbox;
};

#endif // FATALERRORDIALOG_H
