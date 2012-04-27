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
#ifndef BUTTON_H
#define BUTTON_H

#include "Inputs.h"
#include "list.h"
#include "object.h"
#include "shape.h"

// the default button class has no concept of screen coords or points.
// im leaving that stuff to be handled in subclasses so there can be buttons with different shaps like circles and ovals

class Button : public Object
{
public:
	Button();

	enum Trigger
	{
		Btn_None,
		Btn_Up,
		Btn_Down,
		Btn_Left,
		Btn_Right,
		Btn_A,
		Btn_B,
		Btn_Plus,
		Btn_Minus,
		Btn_Home
	};

	// set specific animations for events
	virtual void SetIdleAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop = false, bool forwardAndReverse = false );
	virtual void SetMouseOverAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop = false, bool forwardAndReverse = false );
	virtual void SetMouseOutAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop = false, bool forwardAndReverse = false );
	virtual void SetClickAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop = false, bool forwardAndReverse = false );
	virtual void SetHeldAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop = false, bool forwardAndReverse = false );
	virtual void SetReleaseAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop = false, bool forwardAndReverse = false );

	// set a trigger to scan for when checking for button clicks
	virtual void SetTrigger( Trigger trig ){ trigger = trig; }

	// disable responding to the user input
	void SetEnabled( bool enable );
	bool IsEnabled(){ return enabled; }

	// update with controler input
	virtual void Update();
	virtual bool Contains( float x __attribute__((unused)),  float y __attribute__((unused)) ) const { return false; }


	enum State
	{
		St_Idle,
		St_Over,
		St_Held,
		St_Clicked,
		St_Released
	};
	// for overriding whatever state the button thinks it should be in
	void SetState( State st){ state = st; }

	// signals
	sigslot::signal0<> Clicked;
	sigslot::signal0<> Held;
	sigslot::signal0<> Released;
	sigslot::signal0<> MouseOver;
	sigslot::signal0<> MouseOut;
	sigslot::signal0<> SwitchedToIdleAnimation;

protected:
	struct EventAnimation
	{
		Animation *anim;
		FrameNumber start;
		FrameNumber end;
		bool loop;
		bool forwardAndReverse;
		EventAnimation(): anim( NULL ), start( 0 ), end( 0 ), loop( false ), forwardAndReverse( false ){}
	};

	// animations to be played
	EventAnimation idleAnim;
	EventAnimation mouseOverAnim;
	EventAnimation mouseOutAnim;
	EventAnimation clickAnim;
	EventAnimation heldAnim;
	EventAnimation releaseAnim;

	State state;
	Trigger trigger;
	bool enabled;

	void AnimateNowOrLater( const std::string &brlan, FrameNumber start = 0.f,
							FrameNumber end = -1.f, FrameNumber wait = -1.f,
							bool loop = true, bool forwardAndReverse = false );

};

// button that is shaped like a, um, circle
class CircleButton : public Button, public Circle
{
public:
	CircleButton( u32 x = 0, u32 y = 0, u32 r = 0 )
		: Circle( r, x, y )
	{
	}
	virtual bool Contains( float x, float y ) const { return Circle::Contains( x, y ); }
};

// button that has 4 corners
class QuadButton : public Button, public Quad
{
public:
	QuadButton( u32 x = 0, u32 y = 0, u32 w = 0, u32 h = 0 )
		: Quad( x, y, w, h )
	{
	}
	virtual bool Contains( float x, float y ) const { return Quad::Contains( x, y ); }
};

#endif // BUTTON_H
