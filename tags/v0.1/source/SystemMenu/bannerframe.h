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
#ifndef BANNERFRAME_H
#define BANNERFRAME_H

#include "Banner.h"
#include "button.h"
#include "object.h"
#include "systemmenuresource.h"

// this is a class to deal with the frame the goes over the large banners
class BannerFrame :public SystemMenuResource
{
public:
	BannerFrame();
	~BannerFrame();

	bool Load( const u8* chanTtlAshData, u32 chanTtlAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen, u8 render_alpha = 0xFF );

	void ResetButtons();

	// signals

	// these just forward signals from the 2 main buttons
	sigslot::signal0<> LeftBtnClicked;
	sigslot::signal0<> RightBtnClicked;

private:
	Layout *channelFrame;
	QuadButton *leftBtn;
	QuadButton *rightBtn;

	std::map< std::string, Animation *>brlans;

	// handle different states
	enum State
	{
		St_Unk,
		St_Idle
	};
	State state;
	void UpdateState();

	void SetupButtons();

	// private slots
	void LeftButtonClickedSlot();
	void RightButtonClickedSlot();

	void LeftBtnAnimDone();
	void RightBtnAnimDone();

	int clicked;
};

#endif // BANNERFRAME_H
