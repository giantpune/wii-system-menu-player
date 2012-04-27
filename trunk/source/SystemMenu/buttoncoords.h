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
#ifndef BUTTONCOORDS_H
#define BUTTONCOORDS_H

#include <gctypes.h>

// get some coords for buttons

enum SystemMenuButton
{
	CmnBtn_Settings,		// the main settings button
	CmnBtn_Mail,			// the main mail one
	CmnBtn_LeftArrowSM,		// the main left and right arrows, small and large
	CmnBtn_RightArrowSM,
	CmnBtn_LeftArrowLG,
	CmnBtn_RightArrowLG,

	ChanSel_0,				// the 12 buttons for the channel grid
	ChanSel_1,				// 0 - 3 is the top row, 4 - 7 is the middle one,
	ChanSel_2,				// and 8 - 11 is the bottom row
	ChanSel_3,
	ChanSel_4,
	ChanSel_5,
	ChanSel_6,
	ChanSel_7,
	ChanSel_8,
	ChanSel_9,
	ChanSel_10,
	ChanSel_11,

	ChanTtl_L,				// the 2 buttons at the bottom of the screen
	ChanTtl_R,				// when the big banner is playing

	// buttons for the dialog windows
	Dlg_A1_Btn1,

	Dlg_A2_Btn1,
	Dlg_A2_Btn2,

	Dlg_A3_Btn1,
	Dlg_A3_Btn2,
	Dlg_A3_Btn3,

	Dlg_B_Btn1,
	Dlg_B_Btn2,

	// buttons for the settings
	Setup_Back,

	Setup_Sel_Left_SM,
	Setup_Sel_Right_SM,
	Setup_Sel_Left_LG,
	Setup_Sel_Right_LG,

	// buttons for the grid of save icons
	SaveGrid_0,
	SaveGrid_1,
	SaveGrid_2,
	SaveGrid_3,
	SaveGrid_4,

	SaveGrid_5,
	SaveGrid_6,
	SaveGrid_7,
	SaveGrid_8,
	SaveGrid_9,

	SaveGrid_10,
	SaveGrid_11,
	SaveGrid_12,
	SaveGrid_13,
	SaveGrid_14,

	// for the tabs in the upper right of the save management
	SaveTab_1,
	SaveTab_2,

	// for the copy/delete save dialog
	SaveDlg_1,
	SaveDlg_2,

	// for the move/copy/delete dialog started in 4.0-ish
	// earlier system menus use the same buttons as the save dialog ( 2 buttons copy/delete )
	ChanEditDlg_Move,
	ChanEditDlg_Copy,
	ChanEditDlg_Delete,

	// buttons in the delete/move channel grid
	ChannelEdit_0,
	ChannelEdit_1,
	ChannelEdit_2,
	ChannelEdit_3,
	ChannelEdit_4,

	ChannelEdit_5,
	ChannelEdit_6,
	ChannelEdit_7,
	ChannelEdit_8,
	ChannelEdit_9,

	ChannelEdit_10,
	ChannelEdit_11,
	ChannelEdit_12,
	ChannelEdit_13,
	ChannelEdit_14,

	// buttons used for the home menu
	HBMTopBar,
	HBMBottomBar,
	HBMWiimoteDown,
	HBMWiimoteUp,
	HBMCenterBtn


};


// this gets 4 coords for buttons, but some require less
//! like circles, the only need x, y, and radius
//! im making it return the radius as w, and h  will be 0
void ButtonCoords( SystemMenuButton button, u32 &x, u32 &y, u32 &w, u32 &h );

#endif // BUTTONCOORDS_H
