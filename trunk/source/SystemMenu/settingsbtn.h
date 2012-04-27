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
#ifndef SETTINGSBTN_H
#define SETTINGSBTN_H

#include "button.h"
#include "object.h"
#include "systemmenuresource.h"
#include "utils/char16.h"

class SettingsBtn: public SystemMenuResource
{
public:
	SettingsBtn();
	~SettingsBtn();

	bool Load( const u8* setupBtnAshData, u32 setupBtnAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	// the back button has been clicked and its animation is done
	sigslot::signal0<> BackClicked;

	// show/hide the "Wii" logo in the upper right
	void HideWiiLogo( bool hide );

	void DisableBackBtn( bool disable );

protected:
	Layout *layout;
	QuadButton *btn;
	Object *wiiObj;
	Object *cmnObj;

	std::map< std::string, Animation *>brlans;

	// used to delay sending the "CLick" of the button until it has finished its animation
	bool clicked;
	void BackAnimDone();
	void BackBtnClicked();
};




#endif // SETTINGSBTN_H
