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
#ifndef SAVEGRID_H
#define SAVEGRID_H

#include "button.h"
#include "char16.h"
#include "dialogwindow.h"
#include "object.h"
#include "systemmenuresource.h"

class SaveGrid: public SystemMenuResource
{
public:
	SaveGrid();
	~SaveGrid();

	bool Load( const u8* memoryAshData, u32 memoryAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );
	void SetDialog( DialogWindow *yesNoDialog );

	// signals to start copying data
	//! u64 = tid
	//! u8 = direction.  1 means wii -> SD, 2 means SD -> Wii
	sigslot::signal2< u64, u8 > StartCopying;

	// slot to receive copying status
	//! not sure yet what the status codes will be.  for now, 0 means success
	void CopyFinished( u32 status );


	void SetPage( u16 page );

	// the back button is shared with the setupSel.ash
	void BackButtonClicked();

	// sent when the back button is clicked and the user wants to exit this menu
	sigslot::signal0<> Done;

	// sent when the back button sdould be disabled/enabled
	sigslot::signal1< bool > DisableBackButton;

	// used to let this resources know to ignore the back button
	void SetHidden( bool hide );




protected:
	DialogWindow *dialogB2;
	Layout *dialogLyt;
	Layout *gridLyt;
	//Layout *gridBtnLyt;// used for the pages that aren't clickable

	// buttons for the dialog
	QuadButton *dialogBtn1;
	QuadButton *dialogBtn2;

	// buttons for the arrows
	//QuadButton *btnArrowLeft;
	//QuadButton *btnArrowRight;

	// buttons for the tabs at the top right
	QuadButton *btnTabWii;
	QuadButton *btnTabSD;

	// object for animating the grid
	Object *gridObj;
	Object *gridEntryObj;// used to fade in all the grid squares

	// object for animating the dialog
	Object *dlgObj;

	struct SaveGridItem
	{
		Layout *lyt;
		u8* lytData;
		SaveGridItem(): lyt( new Layout ),lytData ( NULL )
		{
		}
		~SaveGridItem()
		{
			delete lyt;
			free( lytData );
		}
	};

	class SaveGridBtn: public sigslot::has_slots<>
	{
	public:
		Animation *brlanMOver;
		Animation *brlanMOut;
		Animation *brlanClick;
		QuadButton *btn;
		u32 idx;
		SaveGridBtn()
			: brlanMOver( NULL ),
			  brlanMOut( NULL ),
			  brlanClick( NULL ),
			  btn( new QuadButton ),
			  clicked( false )
		{
			btn->Clicked.connect( this, &SaveGrid::SaveGridBtn::ButtonClicked );
			btn->Finished.connect( this, &SaveGrid::SaveGridBtn::ButtonAnimDone );
		}
		sigslot::signal1< u32 >Clicked;
	protected:
		bool clicked;
		void ButtonClicked(){ clicked = true; }
		void ButtonAnimDone(){ if( clicked ){ clicked = false; Clicked( idx ); } }
	};
	SaveGridBtn gridBtns[ 15 ];
	SaveGridItem gridLyts[ 45 ];

	std::map< std::string, Animation *>brlans;

	char16 freeSpacebuffer[ 0x20 ];
	char16 promptBlocksBuffer[ 8 ];
	void SetupText();

	enum State
	{
		St_FadeIn,
		St_Idle,
		St_ShitfLeft,
		St_ShitfRight,

		St_Dialog,

		St_Hidden
	};
	enum Mode
	{
		M_Wii,
		M_SD
	};
	enum DlgState
	{
		DSt_FadeIn,
		DSt_FadeOut,
		DSt_IdleCopyDel,

		DSt_ConfirmCopyIn,
		DSt_ConfirmCopyOut,
		DSt_ConfirmCopyIdle,
		DSt_Copying,

		DSt_CantCopy			// for whatever reason, the save cant be copied
	};

	State state;
	Mode mode;
	DlgState dlgState;

	bool LoadDialog( const U8Archive &arc );

	// slots
	void GridAnimDone();
	void DialogAnimDone();
	void WiiBtnClicked();
	void SDBtnClicked();
	void LeftBtnClicked();
	void RightBtnClicked();

	u16 currentPage;

	void MaybeShiftToNextPage();
	void MaybeShiftToPrevPage();

	void GridBtnClicked( u32 idx );

	void PromptBtn1AnimDone();
	void PromptBtn2AnimDone();
	void PromptBtn1Clicked();
	void PromptBtn2Clicked();

	// set text messages

	// setup prompt for selected save
	u32 selectedItem;
	void SetupPromptForSelectedSave();

	// respond to yes/no dialog
	void YesOrNoClicked( u8 btn );

	// update the "free space" text
	void UpdateFreeSpaceText();

};


#endif // SAVEGRID_H
