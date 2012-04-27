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
#ifndef HOMEMENU_H
#define HOMEMENU_H

#include "button.h"
#include "char16.h"
#include "systemmenuresource.h"

// class for dealing with the home-menu
class HomeMenu: public SystemMenuResource
{
public:
	HomeMenu();
	~HomeMenu();

	bool Load( const u8* homeBtn1AshData, u32 homeBtn1AshSize, const u8* lang_homeBtn1AshData, u32 lang_homeBtn1AshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );
	//void SetBGTexture( Texture *tex );

	void Reset();

	// sent when this menu is done (the menu is canceled)
	sigslot::signal0<> Done;

	// sent when the menu is done and the user wants to exit
	sigslot::signal0<> ExitToWiiMenu;

protected:
	// main layout.  it comes with its own cursor layouts, but we can ignore those
	Layout *lyt;

	// buttons
	QuadButton *btn1;
	QuadButton *btnTopBar;
	QuadButton *btnBottomBar;
	QuadButton *btnWiimote;

	QuadButton *btnDlg1;
	QuadButton *btnDlg2;

	// objects
	Object *testObj;
	Object *dlgObj;

	// batteries
	Object *btryObj[4];
	int btryLevel[4];

	std::map< std::string, Animation *>brlans;

	enum State
	{
		St_FadeIn,			// initial fade in
		St_BtnIdle,			// showing teh certer button and iwing for something to be clicked
		St_FadeOut,

		St_Dialog			// the dialog window is doing something
	};
	State state;

	enum DlgState
	{
		DSt_FadeIn,
		DSt_Idle,
		DSt_FadeOutYes,
		DSt_FadeOutNo
	};
	DlgState dlgState;

	// keeping track of which buttons are pressed and shit
	enum Choice
	{
		Ch_None,
		Ch_TopBar,
		Ch_BottomBar,
		Ch_CenterBtn,
		Ch_DlgLeft,
		Ch_DlgRight
	};
	Choice choice;

	// buffer to hold the text for the prompt
	char16 dlgTxt[ 30 ];


	// the images are spread out across 2 u8 archives, so loading them up is a bit of a bitch
	bool LoadTpl( Texture *tex, const U8Archive &arc );

	// slots
	void TopBarAnimDone();
	void TopBarClicked();
	void CenterBtnClicked();
	void CenterBtnAnimDone();
	void DialogAnimDone();

	void DlgBtn1Clicked();
	void DlgBtn1AnimDone();
	void DlgBtn2Clicked();
	void DlgBtn2AnimDone();

};



#endif // HOMEMENU_H
