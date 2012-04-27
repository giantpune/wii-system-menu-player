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
#ifndef DISCCHANNEL_H
#define DISCCHANNEL_H


#include "object.h"
#include "systemmenuresource.h"
#include "utils/char16.h"


// this is the big disc channel, not the icon

class DiscChannel: public SystemMenuResource
{
public:
	DiscChannel();
	~DiscChannel();

	bool Load( const u8* diskBannAshData, u32 diskBannAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	// sets everything back to the initial fadein animations
	void Reset();

	// jump imediately to specific animations
	void JumpToUnknownDiscAnim();

	// starts the animation where both disc are spinning
	void StartReadingDisc();

	// should be called after the disc is already being read
	void DiscIsWii();
	void DiscIsGC();
	void DiscIsUnknown();
	void EjectDisc();

	// sent whenever either of the "insert disc" animations are completed
	sigslot::signal0<> Finished;


protected:
	Layout *layout;
	std::map< std::string, Animation *>brlans;

	Object *testObj;
	Object *wiiDisc;
	Object *gcDisc;

	enum State
	{
		St_Idle_Nodisc,
		St_DiscSpinLoop,
		St_GCFadeOut,			// wii disc detected, fading out the GC disc
		St_WiiFadeOut,			// gc disc detected, fading out the Wii disc
		St_FadeoutWiiAndGC,		// unknown disk inserted, start fading out both wii and GC images
		St_InsertWiiDisc,
		St_InsertGCDisc,
		St_Unknown,				// just sitting there with the idle disc animation showing
		St_FadeOutUnknown,
		St_EjectingDisc
	};
	State state;

	//void EnterDiscSpinLoop();
	void GCDiscAnimDone();
	void WiiDiscAnimDone();
	void BackgroundAnimDone();
	void WiiDiscAnimStarted( AnimationLink * );
	void GCDiscAnimStarted( AnimationLink * );
	void BackgroundAnimStarted( AnimationLink * );

};





#endif // DISCCHANNEL_H
