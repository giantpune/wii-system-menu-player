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
#ifndef CHANNELGRID_H
#define CHANNELGRID_H


#include "button.h"
#include "dihandler.h"
#include "discchannelicon.h"
#include "object.h"
#include "systemmenuresource.h"

// class to handle the grid that contains all the small channel icons
class ChannelGrid : public SystemMenuResource
{
public:
	ChannelGrid();
	~ChannelGrid();

	bool Load( const u8* chanSelAshData, u32 chanSelAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );
	void SetDiscChannelPointer( DiscChannelIcon * dcIcon );

	// expose the buttonsso they can be connected to something
	QuadButton *ChannelButton( u8 idx ) { return idx < channelHighlites.size() ? channelHighlites[ idx ] : NULL; }

	// call this when the banner has been read from the disc and parsed
	void SetDiscChannelIcon( Layout *lyt, Object *obj );

	// slot

	// ask that the grid shift left or right.  it might just ignore the request if it doesn't
	// think it should be shifting
	void RequestShiftLeft();
	void RequestShiftRight();

	// call this whenever the list of channels changes
	void ChannelListChanged();

	// making this a slot so it can be triggered from a thread or something else
	// this way we dont update it every single frame when it isnt needed
	void UpdateClock();

	void DiscInserted( DiHandler::DiscType dt );
	void DiscEjected();

	// signals

	// sent when entering/leaving the first and last page
	// i put these here to let the button panel know when it should hide the
	// arrow buttons
	sigslot::signal1< bool >FirstPage;
	sigslot::signal1< bool >LastPage;

	// sent when a channel is clicked
	//! column, row, index
	//! index -1 is the disc channel
	sigslot::signal3< u8, u8, int >ChannelClicked;

	// sent when the clock changes to the next day
	//! weekday, date, month
	sigslot::signal3< u8, u8, u8 >DateChanged;

	// jumps right to a page without any scrolling action
	void SetPage( u32 newPage );
	bool GetIconPaneCoords( int selected, f32 *paneX1 = NULL, f32 *paneY1 = NULL, f32 *paneX2 = NULL, f32 *paneY2 = NULL );

	// unbinds all the channels so we can edit the channels in the settings
	void UnbindAllChannels();

    // get icon pane
    Pane *IconPane(int i) const { if(i < 0 || i >= (int) iconPanes.size()) return NULL; return iconPanes[i]; }

private:
	DiscChannelIcon *dcIcon;// pointer to the resource for the disc channel icons
	time_t lastTime;
	Layout *channelFrame;
	Layout *channelStatic;
	Layout *clockLyt;
	List< Layout * >highliteLyts;
	u8* highliteBuffer[ 12 ];
	u8* highliteBrlanBufferMouseOver[ 12 ];
	u8* highliteBrlanBufferMouseOut[ 12 ];
	u8* highliteBrlanBufferClick[ 12 ];
	List< Animation * >highliteAnims;

	Object *gridObj;
	Object *clockObj;
	Object *staticObj;
	List< QuadButton * >channelHighlites;

	// these are where the channels show up
	//! they will get delete by the layout when it is deleted, so no need to do it
	List< Pane * >iconPanes;

	std::map< std::string, Animation *>brlans;

	// handle different states
	enum State
	{
		St_Unk,
		St_IdleDown,
		St_IdleUp,
		St_ShiftUp,
		St_ShiftDown,
		St_ShiftLeft,
		St_ShiftRight
	};

	enum DcState
	{
		DSt_None,
		DSt_Wii,
		DSt_GC
	};

	State state;


	DcState dcState;
	Layout *dcLayout;
	Object *dcObj;



	void DeleteEverything();

	void HandleUserInput();

	// used to hide the far left/right pages that cant be accessed
	//! page 1 is the left and 3 is the right
	void HidePage( u8 page, bool hide );

	// number of pages
	int pageCnt;

	// current page
	int pageNo;

	// private slots
	void ChannelClicked0();
	void ChannelClicked1();
	void ChannelClicked2();
	void ChannelClicked3();
	void ChannelClicked4();
	void ChannelClicked5();
	void ChannelClicked6();
	void ChannelClicked7();
	void ChannelClicked8();
	void ChannelClicked9();
	void ChannelClicked10();
	void ChannelClicked11();
	void GridAnimationFinished();
};


#endif // CHANNELGRID_H
