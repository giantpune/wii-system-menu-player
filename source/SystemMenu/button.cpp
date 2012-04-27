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
#include "button.h"

Button::Button()
	: state( St_Idle ),
	  trigger( Btn_None ),
	  enabled( true )
{
}

void Button::SetIdleAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop, bool forwardAndReverse )
{
	idleAnim.anim = anim;
	idleAnim.start = start;
	idleAnim.end = end;
	idleAnim.loop = loop;
	idleAnim.forwardAndReverse = forwardAndReverse;
}

void Button::SetMouseOverAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop, bool forwardAndReverse )
{
	mouseOverAnim.anim = anim;
	mouseOverAnim.start = start;
	mouseOverAnim.end = end;
	mouseOverAnim.loop = loop;
	mouseOverAnim.forwardAndReverse = forwardAndReverse;
}

void Button::SetMouseOutAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop, bool forwardAndReverse )
{
	mouseOutAnim.anim = anim;
	mouseOutAnim.start = start;
	mouseOutAnim.end = end;
	mouseOutAnim.loop = loop;
	mouseOutAnim.forwardAndReverse = forwardAndReverse;
}

void Button::SetClickAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop, bool forwardAndReverse )
{
	clickAnim.anim = anim;
	clickAnim.start = start;
	clickAnim.end = end;
	clickAnim.loop = loop;
	clickAnim.forwardAndReverse = forwardAndReverse;
}

void Button::SetHeldAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop, bool forwardAndReverse )
{
	heldAnim.anim = anim;
	heldAnim.start = start;
	heldAnim.end = end;
	heldAnim.loop = loop;
	heldAnim.forwardAndReverse = forwardAndReverse;
}

void Button::SetReleaseAnimation( Animation *anim, FrameNumber start, FrameNumber end, bool loop, bool forwardAndReverse )
{
	releaseAnim.anim = anim;
	releaseAnim.start = start;
	releaseAnim.end = end;
	releaseAnim.loop = loop;
	releaseAnim.forwardAndReverse = forwardAndReverse;
}

void Button::AnimateNowOrLater( const std::string &brlan, FrameNumber start, FrameNumber end, FrameNumber wait, bool loop, bool forwardAndReversee )
{
	if( 0 )
	{
		ScheduleAnimation( brlan, start, end, wait, loop, forwardAndReversee );
	}
	else
	{
		SetAnimation( brlan, start, end, wait, loop, forwardAndReversee );
	}
}

void Button::SetEnabled( bool enable )
{
	enabled = enable;
	if( idleAnim.anim && enable )
	{
		ScheduleAnimation( idleAnim.anim->GetName(), idleAnim.loop ? idleAnim.start : idleAnim.end - 1,
						   idleAnim.end, -1.f, idleAnim.loop, idleAnim.forwardAndReverse );
	}
}

void Button::Update()
{
	State newState = St_Idle;

	for(int i = 3; i >= 0 && enabled; i--)
	{
		const Controller &pad = Pad( i );
		if( pad.IsTaken() )
		{
			continue;
		}
		const WPADData &wpad = pad.GetData();

		// determine new state
		if( wpad.ir.valid && Contains( wpad.ir.x, wpad.ir.y ) )
		{
			switch( trigger )
			{
			case Btn_Up:	newState = pad.hUp() ? St_Held : pad.pUp() ? St_Clicked : St_Over; break;
			case Btn_Down:	newState = pad.hDown() ? St_Held : pad.pDown() ? St_Clicked : St_Over; break;
			case Btn_Left:	newState = pad.hLeft() ? St_Held : pad.pLeft() ? St_Clicked : St_Over; break;
			case Btn_Right:	newState = pad.hRight() ? St_Held : pad.pRight() ? St_Clicked : St_Over; break;
			//case Btn_A:		newState = ( pad.hA() ? ( pad.pA() ? St_Clicked : St_Held ) : St_Over ); break;

				// meh, i don't need holdable buttons right now
			case Btn_A:		newState = pad.pA() ? St_Clicked : St_Over; break;
			case Btn_B:		newState = pad.hB() ? St_Held : pad.pB() ? St_Clicked : St_Over; break;
			case Btn_Plus:	newState = pad.hPlus() ? St_Held : pad.pPlus() ? St_Clicked : St_Over; break;
			case Btn_Minus:	newState = pad.hMinus() ? St_Held : pad.pMinus() ? St_Clicked : St_Over; break;
			case Btn_Home:	newState = pad.hHome() ? St_Held : pad.pHome() ? St_Clicked : St_Over; break;
			default:
				newState = St_Over;
				break;
			}
			//gprintf( "newstate: %u oldstate: %u\n", newState, state );
			Pad( i ).Take();
		}
	}

	// nothing to do
	if( newState == state )
	{
		return;
	}
	//gprintf( "newstate: %u oldstate: %u\n", newState, state );

	// handle new state
	switch( newState )
	{
	case St_Idle:
	{
		switch( state )
		{
		case St_Clicked:
		case St_Over:
		{
			if( mouseOutAnim.anim )
			{
				AnimateNowOrLater( mouseOutAnim.anim->GetName(), mouseOutAnim.start, mouseOutAnim.end, -1.f, false );
			}
			MouseOut();
		}
		break;
		// dunno how it would get to idle state from anything except over
		default:
			break;
		}

		// set idle animation to play after the mouseout one is done
		if( idleAnim.anim )
		{
			AnimateNowOrLater( idleAnim.anim->GetName(), idleAnim.loop ? idleAnim.start : idleAnim.end - 1,
							   idleAnim.end, -1.f, idleAnim.loop, idleAnim.forwardAndReverse );
		}

		// set state to idle
		state = St_Idle;
	}
	break;
	case St_Over:
	{
		bool scheduleIdle = false;
		switch( state )
		{
		case St_Idle:
		{
			// play mouseover animation
			if( mouseOverAnim.anim )
			{
				AnimateNowOrLater( mouseOverAnim.anim->GetName(), mouseOverAnim.start, mouseOverAnim.end, -1.f, false );
			}
			MouseOver();
		}
		break;
		case St_Held:
		{
			// play released animation
			if( releaseAnim.anim )
			{
				AnimateNowOrLater( releaseAnim.anim->GetName(), releaseAnim.start, releaseAnim.end, -1.f, false );
			}
			// set over animation next
			if( mouseOverAnim.anim )
			{
				ScheduleAnimation( mouseOverAnim.anim->GetName(), mouseOverAnim.loop ? mouseOverAnim.start : mouseOverAnim.end - 1,
								   mouseOverAnim.end, -1.f, mouseOverAnim.loop, mouseOverAnim.forwardAndReverse );
			}
			else
			{
				scheduleIdle = true;
			}
			Released();
		}
		break;
		default:
			break;
		}

		if( scheduleIdle && idleAnim.anim )// no mouseover animation, just use the idle one
		{
			AnimateNowOrLater( idleAnim.anim->GetName(), idleAnim.loop ? idleAnim.start : idleAnim.end - 1,
							   idleAnim.end, -1.f, idleAnim.loop, idleAnim.forwardAndReverse );
		}

		// set state to over
		state = St_Over;
	}
	break;
	case St_Held:
	{
		switch( state )
		{
		case St_Over:
		{
			// play held animation
			if( heldAnim.anim )
			{
				AnimateNowOrLater( heldAnim.anim->GetName(), heldAnim.start, heldAnim.end, -1.f, false );
			}
			Held();
		}
		break;
		default:// i guess it can only get to held state from over state
			break;
		}
		state = St_Held;
	}
	break;
	case St_Clicked:
	{
		switch( state )
		{
		case St_Over:
		{
			// play clicked animation
			if( clickAnim.anim )
			{
				ScheduleAnimation( clickAnim.anim->GetName(), clickAnim.start, clickAnim.end, -1.f, false );
			}
			Clicked();
		}
		break;
		default:// i guess it can only get to held state from over state
			break;
		}
		// just leave state as whatever it already was before the click
	}
	break;
	default:
		break;
	}
	Start();
}
