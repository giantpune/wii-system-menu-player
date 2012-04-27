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
#ifndef BUTTONPANEL_H
#define BUTTONPANEL_H

#include "button.h"
#include "object.h"
#include "systemmenuresource.h"
#include "U8Archive.h"

// handles the buttons across the bottom of the system menu
// and message board
class ButtonPanel : public SystemMenuResource
{
public:
	ButtonPanel();
	~ButtonPanel();

	bool Load( const u8* cmnBtnAshData, u32 cmnBtnAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );
	void UpdateInput();

	// hide all the buttons except the left/right arrows
	void ShowArrowsOnly( bool hideEverythingElse = true );

	// slots
	void HideLeftArrow( bool hide );
	void HideRightArrow( bool hide );

	// expose a couple buttons for connecting signals
	QuadButton *LeftArrow(){ return leftArrow; }
	QuadButton *RightArrow(){ return rightArrow; }

	sigslot::signal0<> SettingsBtnClicked;

protected:
	Layout *layout;
	QuadButton *leftArrow;
	QuadButton *rightArrow;
	CircleButton *settingsBtn;
	CircleButton *mailBtn;

	std::map< std::string, Animation *>brlans;

	enum State
	{
		St_Unk,
		St_IdleGrid
	};
	State state;
	void UpdateState();
	bool CreateButtons( const U8Archive &arc );


	// private slots
	void LeftArrowMouseOver();
	void LeftArrowMouseOut();
	void RightArrowMouseOver();
	void RightArrowMouseOut();

	// used to synchronize the idle animations
	void LeftArrowAnimStarted( AnimationLink *anim );
	void RightArrowAnimStarted( AnimationLink *anim );

	void SettinsgBtnClicked();
	void SettinsgBtnAnimDone();

	int clicked;


};

#endif // BUTTONPANEL_H
