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
#include "buttoncoords.h"

#include "gecko.h"
#include "sc.h"
#include "video.h"


// which button, then x, y, w, and h for widescreen/fullscreen and 480/576
#define CASE_BTN( btn, x_4x3, x_16x9, y_480, y_576, w_4x3, w_16x9, h_480, h_576 )	\
	case btn:																		\
	{																				\
		x = widescreen ? x_16x9 : x_4x3;											\
		w = widescreen ? w_16x9 : w_4x3;											\
		y = pal576 ? y_576 : y_480;													\
		h = pal576 ? h_576 : h_480;													\
	}																				\
	break

void ButtonCoords( SystemMenuButton button, u32 &x, u32 &y, u32 &w, u32 &h )
{
	bool widescreen = ( _CONF_GetAspectRatio() > 0 );
	bool pal576 = ( _CONF_GetVideo() == CONF_VIDEO_PAL && _CONF_GetEuRGB60() < 1 );


	switch( button )
	{
	CASE_BTN( CmnBtn_Mail,			553, 564, 398, 422, 38, 37, 0, 0 );
	CASE_BTN( CmnBtn_Settings,		88, 75, 398, 422, 37, 38, 0, 0 );

	CASE_BTN( CmnBtn_LeftArrowSM,	47, 42, 169, 195, 19, 15, 38, 38 );
	CASE_BTN( CmnBtn_RightArrowSM,	576, 583, 169, 195, 19, 15, 38, 38 );

	CASE_BTN( CmnBtn_LeftArrowLG,	47, 42, 149, 175, 87, 72, 77, 77 );
	CASE_BTN( CmnBtn_RightArrowLG,	508, 526, 149, 175, 87, 72, 77, 77 );


	CASE_BTN( ChanTtl_L,			74, 114, 375, 400, 232, 188, 59, 77 );
	CASE_BTN( ChanTtl_R,			324, 328, 375, 400, 232, 188, 59, 77 );


	CASE_BTN( ChanSel_0,			72, 70, 54, 80, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_1,			200, 198, 54, 80, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_2,			328, 326, 54, 80, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_3,			456, 454, 54, 80, 112, 117, 82, 82 );

	CASE_BTN( ChanSel_4,			72, 70, 150, 177, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_5,			200, 198, 150, 177, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_6,			328, 326, 150, 177, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_7,			456, 454, 150, 177, 112, 117, 82, 82 );

	CASE_BTN( ChanSel_8,			72, 70, 246, 274, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_9,			200, 198, 246, 274, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_10,			328, 326, 246, 274, 112, 117, 82, 82 );
	CASE_BTN( ChanSel_11,			456, 454, 246, 274, 112, 117, 82, 82 );

	// dialog windows
	CASE_BTN( Dlg_A1_Btn1,			233, 233, 339, 361, 339, 174, 55, 55 ); //

	CASE_BTN( Dlg_A2_Btn1,			93, 139, 339, 361, 212, 174, 55, 55 ); //
	CASE_BTN( Dlg_A2_Btn2,			338, 330, 339, 361, 212, 174, 55, 55 ); //

	CASE_BTN( Dlg_A3_Btn1,			136, 136, 202, 202, 0, 368, 58, 58 ); //
	CASE_BTN( Dlg_A3_Btn2,			136, 136, 270, 270, 0, 368, 58, 58 ); //
	CASE_BTN( Dlg_A3_Btn3,			136, 136, 337, 337, 0, 368, 58, 58 ); //

	CASE_BTN( Dlg_B_Btn1,			129, 129, 393, 393, 0, 174, 55, 55 ); //
	CASE_BTN( Dlg_B_Btn2,			341, 341, 393, 393, 0, 174, 55, 55 ); //

	// settings
	CASE_BTN( Setup_Back,			46, 65, 391, 422, 212, 174, 55, 55 ); //
	CASE_BTN( Setup_Sel_Left_SM,	50, 77, 141, 166, 240, 205, 192, 192 ); //
	CASE_BTN( Setup_Sel_Right_SM,	350, 358, 141, 166, 240, 205, 192, 192 ); //
	CASE_BTN( Setup_Sel_Left_LG,	33, 77, 141, 166, 268, 205, 192, 192 ); //
	CASE_BTN( Setup_Sel_Right_LG,	337, 352, 135, 135, 268, 220, 210, 210 ); //

	// save grid
	CASE_BTN( SaveGrid_0,			65, 74, 98, 122, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_1,			172, 181, 98, 122, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_2,			277, 285, 98, 122, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_3,			382, 391, 98, 122, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_4,			489, 498, 98, 122, 76, 60, 74, 74 ); //

	CASE_BTN( SaveGrid_5,			65, 74, 190, 216, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_6,			172, 181, 190, 216, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_7,			277, 285, 190, 216, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_8,			382, 391, 190, 216, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_9,			489, 498, 190, 216, 76, 60, 74, 74 ); //

	CASE_BTN( SaveGrid_10,			65, 74, 282, 305, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_11,			172, 181, 282, 305, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_12,			277, 285, 282, 305, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_13,			382, 391, 282, 282, 76, 60, 74, 74 ); //
	CASE_BTN( SaveGrid_14,			489, 498, 282, 305, 76, 60, 74, 74 ); //

	CASE_BTN( SaveTab_1,			246, 259, 34, 59, 180, 157, 44, 44 ); //
	CASE_BTN( SaveTab_2,			440, 453, 34, 59, 180, 157, 44, 44 ); //

	CASE_BTN( SaveDlg_1,			146, 151, 248, 271, 127, 121, 96, 96 ); //
	CASE_BTN( SaveDlg_2,			367, 369, 248, 271, 127, 121, 96, 96 ); //


	CASE_BTN( ChanEditDlg_Move,		132, 132, 248, 271, 248, 105, 96, 96 ); //
	CASE_BTN( ChanEditDlg_Copy,		267, 267, 248, 271, 248, 105, 96, 96 ); //
	CASE_BTN( ChanEditDlg_Delete,	402, 402, 248, 271, 248, 105, 96, 96 ); //


	CASE_BTN( ChannelEdit_0,		86, 80, 104, 130, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_1,		183, 180, 104, 130, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_2,		279, 283, 104, 130, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_3,		375, 386, 104, 130, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_4,		470, 490, 104, 130, 85, 74, 62, 62 ); //

	CASE_BTN( ChannelEdit_5,		86, 80, 198, 222, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_6,		183, 180, 198, 222, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_7,		279, 283, 198, 222, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_8,		375, 386, 198, 222, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_9,		470, 490, 198, 222, 85, 74, 62, 62 ); //

	CASE_BTN( ChannelEdit_10,		86, 80, 291, 315, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_11,		183, 180, 291, 315, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_12,		279, 283, 291, 315, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_13,		375, 386, 291, 315, 85, 74, 62, 62 ); //
	CASE_BTN( ChannelEdit_14,		470, 490, 291, 315, 85, 74, 62, 62 ); //

	// home menu
	CASE_BTN( HBMTopBar,			0, 0, 0, 0, 640, 640, 93, 110 ); //
	CASE_BTN( HBMBottomBar,			0, 0, 379, 400, 640, 640, 101, 101 ); //
	CASE_BTN( HBMWiimoteDown,		0, 95, 356, 356, 0, 61, 124, 124 ); //
	CASE_BTN( HBMWiimoteUp,			0, 0, 0, 0, 0, 0, 0, 0 ); //
	CASE_BTN( HBMCenterBtn,			188, 210, 185, 203, 266, 220, 100, 100 ); //
	default:
		gprintf( "unhandled button\n" );
		x = 0;
		y = 0;
		w = 0;
		h = 0;
		break;
	}
}

/*

 HBMTopBar,
 HBMBottomBar,
 HBMWiimoteDown,
 HBMWiimoteUp,
 HBMCenterBtn


  channel edit grid
  [80x109] - [150x163]													[386x107] - [461x165]
						[180x199] - [256x257]													[490x198] - [563x258]
												[283x291] - [257x350]



  save grid:
  [74x99] - [134x172]													[391x98] - [452x171]
						[181x190] - [239x266]													[498x190] - [557x265]
												[285x282] - [346x357]



 Setup_Back,

 Setup_Sel_Left_SM,
 Setup_Sel_Right_SM,
 Setup_Sel_Left_LG,
 Setup_Sel_Right_LG



 CmnBtn_Settings,		// the main settings button
 CmnBtn_Mail,			// the main mail one
 CmnBtn_LeftArrow,		// the main left and right arrows
 CmnBtn_RightArrow


 CmnBtn_Mail - top = 574, 372  bottom = 574, 448  left = 546, 407  right = 609, 407
 width = 63  + 6
 height = 76 - 6

 radius = 34
 x = 580
 y = 608

 CmnBtn_Settings:  left = 35 right = 94


 CmnBtn_LeftArrowSM  left = 30   right = 45   top = 169  bottom = 207			width = 15  height = 38
 CmnBtn_RightArrowSM  left = 598   right = 613   top = 169  bottom = 226

 CmnBtn_LeftArrowLG  left = 30   right = 99   top = 149  bottom = 226			width = 66  height = 77
 CmnBtn_RightArrowLG  left = 541   right = 613   top = 149  bottom = 226


 ChanTtl_L left = 114  right = 302  top = 375  bottom = 434		width = 188  height = 59
 ChanTtl_R left = 328 right = 522


 ChanSel_## buttons:
 [57, 49] x [178, 131]													[465, 50] x 131]
						[194, 150] x [313, 232]
												[328, 251] x [445, 333]

width = 121
height = 82
 */
