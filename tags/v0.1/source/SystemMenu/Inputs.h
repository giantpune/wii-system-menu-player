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
#ifndef INPUTS_H_
#define INPUTS_H_

#include <wiiuse/wpad.h>

#define MAX_CONTROLS	4

class Controller
{
private:
	WPADData wpad;
	u8 chan;
	float posX;
	float posY;
	float angle;
	u32 lastActivity;
	u16 pad_btns_d;
	u16 pad_btns_h;

	//! allow a button to "take" a conroller for the remainder of a frame so
	//! 2 buttons cant be clicked at once
	bool taken;

	//! Internal functions to check for pressed buttons
	bool pButton(u32 pad_btns, u32 wpad_btns) const {
		return ((pad_btns_d & pad_btns) || (wpad.btns_d & wpad_btns));
	}
	//! Internal functions to check for held buttons
	bool hButton(u32 pad_btns, u32 wpad_btns) const {
		return ((pad_btns_h & pad_btns) || (wpad.btns_h & wpad_btns));
	}
public:
	//! contructor
	Controller(int chan = 0);
	//! Set controller channel
	void SetChannel(int c) { chan = c; }
	int GetChan() const{ return chan; }
	//! Update the controller state
	void Update(void);

	//! basically sets a flag so other buttons will ignore this input
	void Take( bool take = true ){ taken = take; }
	bool IsTaken() const{ return taken; }

	//! Check for pressed buttons
	bool pUp() const {
		return pButton(PAD_BUTTON_UP, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP);
	}
	bool pDown() const {
		return pButton(PAD_BUTTON_DOWN, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN);
	}
	bool pLeft() const {
		return pButton(PAD_BUTTON_LEFT, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT);
	}
	bool pRight() const {
		return pButton(PAD_BUTTON_RIGHT, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT);
	}
	bool pA() const {
		return pButton(PAD_BUTTON_A, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A);
	}
	bool pB() const {
		return pButton(PAD_BUTTON_B, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B);
	}
	bool pPlus() const {
		return pButton(PAD_TRIGGER_R, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS);
	}
	bool pMinus() const {
		return pButton(PAD_TRIGGER_L, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS);
	}
	bool pHome() const {
		return pButton(PAD_BUTTON_START, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME);
	}
	//! Check for held buttons
	bool hUp() const {
		return hButton(PAD_BUTTON_UP, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP);
	}
	bool hDown() const {
		return hButton(PAD_BUTTON_DOWN, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN);
	}
	bool hLeft() const {
		return hButton(PAD_BUTTON_LEFT, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT);
	}
	bool hRight() const {
		return hButton(PAD_BUTTON_RIGHT, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT);
	}
	bool hA() const {
		return hButton(PAD_BUTTON_A, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A);
	}
	bool hB() const {
		return hButton(PAD_BUTTON_B, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B);
	}
	bool hPlus() const {
		return hButton(PAD_TRIGGER_R, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS);
	}
	bool hMinus() const {
		return hButton(PAD_TRIGGER_L, WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS);
	}
	bool hHome() const {
		return hButton(PAD_BUTTON_START, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME);
	}
	//! Get the wpad data -> needed to render pointer
	const WPADData &GetData(void) const { return wpad; }
};

class CInputs
{
public:
	//! Instance class to handle all controller setup and updates
	static CInputs *Instance() { if(!instance) instance = new CInputs(); return instance; }
	static void DestroyInstance() { delete instance; instance = NULL; }
	static s8 WPAD_Stick(WPADData &wpad, u8 right, int axis);
	//! Get the controller data
	Controller &GetController(int chan) { return controller[chan]; }
	//! Update all controller states
	void Update(void);
private:
	CInputs();
	static CInputs *instance;
	Controller controller[MAX_CONTROLS];
};

#define Pad(x)	( CInputs::Instance()->GetController(x) )

#endif
