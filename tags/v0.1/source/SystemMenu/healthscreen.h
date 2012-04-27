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
#ifndef HEALTHSCREEN_H
#define HEALTHSCREEN_H

#include "object.h"
#include "systemmenuresource.h"

// handles the health screen and back menu
class HealthScreen : public SystemMenuResource
{
public:
	HealthScreen();
	~HealthScreen();
	bool Load( const u8* healthAshData, u32 healthAshSize, const u8 *backMenuData, u32 backMenuSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	// skip the health screen and start at the backmenu
	void SkipHealthScreen(){ state = St_Backmenu; }

	// signals
	// sent when they pressed A, if we care for any reason
	sigslot::signal0<> ClickedA;

	// sent when the last animation is done for this whole thing
	sigslot::signal0<> Done;

private:
	Layout *healthLyt;
	Layout *backMenuLyt;
	Object *healthObj;
	Object *backMenuObj;

	std::map< std::string, Animation *>brlans;

	// handle different states
	enum State
	{
		St_Unk,
		St_FadeIn,			// initial fade in
		St_FadeInDone,		// done fading in, waiting to be told to show the "Press A" text
		St_IdlePressA,		// waiting for A to be pressed
		St_PressedA,		// pressed A, waiting for the blink animation to stop so we can bind the fadeout one
		St_FadeOutHealth,	// health screen fading out
		St_Backmenu			// backmenu is running
	};
	State state;
	void UpdateState();
};


#endif // HEALTHSCREEN_H
