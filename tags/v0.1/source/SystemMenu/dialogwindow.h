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
#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H

#include "button.h"
#include "object.h"
#include "systemmenuresource.h"
#include "utils/char16.h"

// dialog A0 is the one shown while updating.  there is a progress bar and a little "activity" animation in the lower right
// dialog A1 has a message and a single button to click
// dialog A2 has a message and 2 buttons to click
// dialog A3 has a message and 3 huge buttons to click
// dialog B has a message and 2 buttons.  the message is written in black text in a grey box at the top of the page.  the buttons
// are on a grey panel at the bottom of the page (seem to remember thas one used to confirm deleting saves and stuff)

class DialogWindow: public SystemMenuResource
{
public:
	DialogWindow();
	~DialogWindow();

	enum Type
	{
		A0,		// progress
		A1,		// message
		A2,		// question with 2 choices
		A3,		// question with 3 choices
		B_1,	// theres really only 1 B dialog, but in the data management, they draw it without the top half, so im treating that
		B_2		// as B_2
	};

	bool Load( const u8* dlgWdwAsh, u32 dlgWdwAshSize, Type type );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );


	// sent when a button was clicked
	// the value will be 0, 1, or 2 depending on which button was clicked
	sigslot::signal1< u8 >ButtonClicked;

	// set the progress used for the A0 dialog
	void SetProgress( u8 prog ){ if( objA0prog && prog < 101 ){ objA0prog->SetFrame( prog ); } }

	// set the text for the current dialog
	void SetText( const char16 *message, const char16* btn1 = NULL, const char16* btn2 = NULL, const char16* btn3 = NULL );

	// set fane in/out
	void SetFadeIn();
	void SetFadeOut();



protected:
	Layout *layoutA0;
	Object *objA0;
	Object *objA0prog;
	Object *objA0wait;

	Layout *layoutA1;
	Object *objA1;
	QuadButton *objA1btn1;

	Layout *layoutA2;
	Object *objA2;
	QuadButton *objA2btn1;
	QuadButton *objA2btn2;

	Layout *layoutA3;
	Object *objA3;
	QuadButton *objA3btn1;
	QuadButton *objA3btn2;
	QuadButton *objA3btn3;

	Layout *layoutB;
	Object *objB;
	QuadButton *objBbtn1;
	QuadButton *objBbtn2;

	std::map< std::string, Animation *>brlans;


	Type type;

	void Button1Clicked();
	void Button2Clicked();
	void Button3Clicked();

	int choice;
	void WindowAnimDone();
};



#endif // DIALOGWINDOW_H
