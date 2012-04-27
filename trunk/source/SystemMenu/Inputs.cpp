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
#include <cstring>
#include <cmath>
#include "utils/tools.h"
#include "Inputs.h"
#include "video.h"

#define PADCAL				50
#define PI					3.14159265f
#define SPEED_CALIBRATION	0.1f
//#define SPEED_CALIBRATION	0.05f// lowered this to make mapping button a bit easier

CInputs *CInputs::instance = NULL;

Controller::Controller(int cont_chan)
	: chan(cont_chan)
	, posX(0.0f)
	, posY(0.0f)
	, lastActivity(0xFFFFFFFF)
	, pad_btns_d(0)
	, pad_btns_h(0)
{
	memset(&wpad, 0, sizeof(WPADData));
}

//! Update the pad data and store it for this loop tick
void Controller::Update(void)
{
	memcpy(&wpad, WPAD_Data(chan), sizeof(WPADData));

	pad_btns_d = PAD_ButtonsDown(chan);
	pad_btns_h = PAD_ButtonsHeld(chan);
	taken = false;

	lastActivity++;

	if(wpad.ir.valid)
	{
		lastActivity = 0;
		posX = wpad.ir.x;
		posY = wpad.ir.y;
		angle = wpad.ir.angle;
	}
	else
	{
		angle = 0.0f;

		int padStickX = PAD_StickX(chan);
		int padStickY = PAD_StickY(chan);

		// GC PAD
		// x-axis
		if(padStickX < -PADCAL)
		{
			posX += (padStickX + PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}
		else if(padStickX > PADCAL)
		{
			posX += (padStickX - PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}
		// y-axis
		if(padStickY < -PADCAL)
		{
			posY -= (padStickY + PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}
		else if(padStickY > PADCAL)
		{
			posY -= (padStickY - PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}

		int wpadX = CInputs::WPAD_Stick(wpad, 0, 0);
		int wpadY = CInputs::WPAD_Stick(wpad, 0, 1);

		// Wii Extensions
		// x-axis
		if(wpadX < -PADCAL)
		{
			posX += (wpadX + PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}
		else if(wpadX > PADCAL)
		{
			posX += (wpadX - PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}
		// y-axis
		if(wpadY < -PADCAL)
		{
			posY -= (wpadY + PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}
		else if(wpadY > PADCAL)
		{
			posY -= (wpadY - PADCAL) * SPEED_CALIBRATION;
			lastActivity = 0;
		}

		if(pad_btns_h || wpad.btns_h)
		//if( wpad.btns_h )
			lastActivity = 0;

		posX = LIMIT(posX, -5.0f, screenwidth+5.0f);
		posY = LIMIT(posY, -5.0f, screenheight+5.0f);

		if(lastActivity < 300) { // (5s on 60Hz and 6s on 50Hz)
			wpad.ir.valid = 1;
			wpad.ir.x = posX;
			wpad.ir.y = posY;
			wpad.ir.angle = angle;
		}
	}
}

//! Init all controllers
CInputs::CInputs()
{
	PAD_Init();
	WPAD_Init();
	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
	for(int i = 0; i < MAX_CONTROLS; i++)
		controller[i].SetChannel(i);
}

//! Update all states
void CInputs::Update(void)
{
	WPAD_ScanPads();
	PAD_ScanPads();

	for(int i = 0; i < MAX_CONTROLS; i++)
		controller[i].Update();
}

//! Nunchuck/Classic Controller stick calculations
s8 CInputs::WPAD_Stick(WPADData &wpad, u8 right, int axis)
{
	float mag = 0.0;
	float ang = 0.0;

	switch ( wpad.exp.type )
	{
		default:
		case WPAD_EXP_NUNCHUK:
		case WPAD_EXP_GUITARHERO3:
		{
			if ( right == 0 )
			{
				mag = wpad.exp.nunchuk.js.mag;
				ang = wpad.exp.nunchuk.js.ang;
			}
			break;
		}
		case WPAD_EXP_CLASSIC:
		{
			if ( right == 0 )
			{
				mag = wpad.exp.classic.ljs.mag;
				ang = wpad.exp.classic.ljs.ang;
			}
			else
			{
				mag = wpad.exp.classic.rjs.mag;
				ang = wpad.exp.classic.rjs.ang;
			}
			break;
		}
	}

	/* calculate x/y value (angle[i] need to be converted into radian) */
	if ( mag > 1.0 )
		mag = 1.0;
	else if ( mag < -1.0 )
		mag = -1.0;

	float val;

	if ( axis == 0 ) // x-axis
		val = (float) (mag * sinf( (PI * ang) / 180.0f ));
	else // y-axis
		val = (float) (mag * cosf( (PI * ang) / 180.0f ));

	return ( s8 )( val * 128.0f );
}
