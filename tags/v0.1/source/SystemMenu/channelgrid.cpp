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
#include <stdio.h>
#include <ctime>
#include "bannerlist.h"
#include "bmg.h"
#include "buttoncoords.h"
#include "channelgrid.h"
#include "sc.h"
#include "utils/tools.h"
#include "video.h"

// array for which banners go on which icon panes
static const s8 iconBindingIdx[ 42 ] =
{
	-22,					// far left page
	-18,
	-14,

	-13, -12, -11, -10,		// left page
	-9, -8, -7, -6,
	-5, -4, -3, -2,

	-1, 0, 1, 2,			// center page, starting with -1 because the disc channel goes there
	3, 4, 5, 6,
	7, 8, 9, 10,

	11, 12, 13, 14,			// right page
	15, 16, 17, 18,
	19, 20, 21, 22,

	23,						// far right page
	27,
	31
};

ChannelGrid::ChannelGrid()
	: dcIcon( NULL ),
	  lastTime( 0 ),
	  channelFrame( NULL ),
	  channelStatic( NULL ),
	  clockLyt( NULL ),
	  gridObj( NULL),
	  clockObj( NULL ),
	  staticObj( NULL ),
	  state( St_Unk ),
	  pageCnt( 1 ),
	  pageNo( 0 )
{
	for( int i = 0; i < 12; i++ )
	{
		highliteBuffer[ i ] = NULL;
		highliteBrlanBufferClick[ i ] = NULL;
		highliteBrlanBufferMouseOver[ i ] = NULL;
		highliteBrlanBufferMouseOut[ i ] = NULL;
	}

	//gprintf( "loaded %u banners\n", bannerList.size() );
}

ChannelGrid::~ChannelGrid()
{
	DeleteEverything();
}

void ChannelGrid::SetDiscChannelPointer( DiscChannelIcon * dcIconPointer )
{
	dcIcon = dcIconPointer;

	// get layout & animation object for the empty disc channel
	dcLayout = dcIcon->GetNoDiscLayout();
	dcObj = dcIcon->GetNoDiscObject();
}

void ChannelGrid::UnbindAllChannels()
{
	for(int i = 0; i < (int) bannerList.size(); i++)
	{
		BannerAsync::RemoveBanner(bannerList[i]->banner);
		bannerList[i]->banner = NULL;
		bannerList[i]->IsBound = false;
	}

	foreach( Pane *pane, iconPanes )
	{
		pane->UnbindAllLayouts();
	}
}

void ChannelGrid::SetPage( u32 newPage )
{
	if( newPage >= (u32)pageCnt )
	{
		gprintf( "pageN >= gridPageCnt\n" );
		return;
	}
	pageNo = newPage;

	int chIdx = pageNo * 12;
	int sIdx = -15;

	//! removed unneeded banners
	for(int i = 0; i < (int) bannerList.size(); i++)
	{
		bool needed = false;
		for( int j = 0; j < 42 && !needed; j++ )
		{
			if( chIdx + iconBindingIdx[ j ] == i )
			{
				needed = true;
			}
		}
		if( !needed )
		{
			BannerAsync::RemoveBanner(bannerList[i]->banner);
			bannerList[i]->banner = NULL;
			bannerList[i]->IsBound = false;
		}
	}

	for( int i = 0; i < 42; i++, sIdx++ )
	{
		Pane *pane = iconPanes[ i ];
		pane->UnbindAllLayouts();

		bool enable = false;

		int whichChannelGoesHere = chIdx + iconBindingIdx[ i ];


		// disc channel
		if( whichChannelGoesHere == -1 )
		{
			enable = true;
			pane->BindLayout( dcLayout, true );
			if( dcState != DSt_None && !pageNo )
			{
				highliteLyts[ 0 ]->FindPane( "RootPane" )->BindLayout( dcIcon->GetInsertDiscLayout(), true );
			}
			else
			{
				highliteLyts[ 0 ]->FindPane( "RootPane" )->UnbindAllLayouts();
			}
		}

		else if( whichChannelGoesHere >= 0 && whichChannelGoesHere < (int)bannerList.size())
		{
			//! Load banner if not loaded yet
			if(!bannerList[ whichChannelGoesHere ]->banner)
			{
				if( !bannerList[ whichChannelGoesHere ]->hbXml )
				{
					//gprintf( "creating normal banner: \"%s\"\n", bannerList[ chIdx ]->filepath.c_str() );
					bannerList[ whichChannelGoesHere ]->banner = new BannerAsync( bannerList[ whichChannelGoesHere ]->filepath );
				}
				else
				{
					//gprintf( "creating homebrew banner: \"%s\"\n", bannerList[ chIdx ]->filepath.c_str() );
					bannerList[ whichChannelGoesHere ]->banner = new BannerAsyncHB( bannerList[ whichChannelGoesHere ]->filepath,
																					bannerList[ whichChannelGoesHere ]->hbXml );
				}
			}

			//! If banner was loaded before bind it right away otherwise later when its loaded async
			if( bannerList[ whichChannelGoesHere ]->banner->getIcon() != NULL )
			{
				enable = true;
				pane->BindLayout( bannerList[ whichChannelGoesHere ]->banner->getIcon(), true );
				bannerList[ whichChannelGoesHere ]->IsBound = true;
			}
			else
			{
				bannerList[ whichChannelGoesHere ]->IsBound = false;
			}
		}

		if( !enable )
		{
			pane->BindLayout( channelStatic, true );
		}

		if( sIdx >= 0 && sIdx < 12 )
		{
			channelHighlites[ sIdx ]->SetEnabled( enable );
		}
	}

	if( !pageNo )
	{
		FirstPage( true );
	}
	if( pageNo >= pageCnt - 1 )
	{
		LastPage( true );
	}
	HidePage( 1, !pageNo );
	HidePage( 3, pageNo >= pageCnt - 1 );
}

bool ChannelGrid::GetIconPaneCoords( int selected, f32 *paneX1, f32 *paneY1, f32 *paneX2, f32 *paneY2 )
{
	selected++;
	int first = pageNo * 12;
	int diff = selected - first;
	if( selected < first || diff > 11 )
	{
		gprintf( "ChannelGrid::GetIconPaneCoords( %i ): out of range\n", selected - 1 );
		return false;
	}
	Pane *pane = iconPanes[ diff + 15 ];

	f32 z;
	f32 AnimPosX1;
	f32 AnimPosY1;
	f32 AnimPosX2;
	f32 AnimPosY2;

	f32 viewport[6];
	f32 projection[7];

	GX_GetViewportv( viewport );
	GX_GetProjectionv( MainProjection, projection, GX_ORTHOGRAPHIC );

	Mtx mv2, mv3;
	guMtxIdentity( mv2 );
	guMtxIdentity( mv3 );
	guMtxTransApply( mv2, mv2, -0.5f * pane->GetOriginX() * pane->GetWidth(),
							 -0.5f * pane->GetOriginY() * pane->GetHeight(), 0.f );
	guMtxTransApply( mv3,mv3, 0.5f * pane->GetOriginX() * pane->GetWidth(),
							 0.5f * pane->GetOriginY() * pane->GetHeight(), 0.f );
	guMtxScaleApply( mv2, mv2, 1.0f, -1.0f, 1.0f );
	guMtxScaleApply( mv3, mv3, 1.0f, -1.0f, 1.0f );
	guMtxConcat( pane->GetView(), mv2, mv2 );
	guMtxConcat( pane->GetView(), mv3, mv3 );

	GX_Project( 0.0f, 0.0f, 0.0f, mv2, projection, viewport, &AnimPosX1, &AnimPosY1, &z );
	GX_Project( 0.0f, 0.0f, 0.0f, mv3, projection, viewport, &AnimPosX2, &AnimPosY2, &z );
	if( paneX1 )
	{
		*paneX1 = AnimPosX1;
	}
	if( paneX2 )
	{
		*paneX2 = AnimPosX2;
	}
	if( paneY1 )
	{
		*paneY1 = AnimPosY1;
	}
	if( paneY2 )
	{
		*paneY2 = AnimPosY2;
	}

	return true;
}

void ChannelGrid::ChannelListChanged()
{
	pageNo = 0;
	pageCnt = (RU( bannerList.size() + 1, 12 )) / 12;
	if( bannerList.size() )
	{
		SetPage( 0 );
	}
}

void ChannelGrid::DeleteEverything()
{
	DELETE( gridObj );
	DELETE( clockObj );
	DELETE( staticObj );

	DELETE( channelFrame );
	DELETE( channelStatic );
	DELETE( clockLyt );

	foreach( Object *obj, channelHighlites )
	{
		delete obj;
	}
	channelHighlites.clear();

	foreach( Layout * l, highliteLyts )
	{
		delete l;
	}
	highliteLyts.clear();

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
	brlans.clear();
	iconPanes.clear();

	for( int i = 0; i < 12; i++ )
	{
		FREE( highliteBuffer[ i ] );
		FREE( highliteBrlanBufferClick[ i ] );
		FREE( highliteBrlanBufferMouseOver[ i ] );
		FREE( highliteBrlanBufferMouseOut[ i ] );
	}

	foreach( Animation *anim, highliteAnims )
	{
		delete anim;
	}
	highliteAnims.clear();
}

void ChannelGrid::UpdateClock()
{
	time_t now;
	struct tm * timeinfo;

	// get current time and see if it has changed enough for us to update the clock
	if( time( &now ) == lastTime )
	{
		return;
	}
	timeinfo = localtime( &now );

	// convert timeinfo into texture indexes
	int mins = timeinfo->tm_min;
	int hours = timeinfo->tm_hour;
	//gprintf( "%.02u:%.02u\n", hours, mins );

	bool pm = ( hours > 11 );
	if( pm )
	{
		hours -= 12;
	}
	else if( !hours )
	{
		hours = 12;
	}

	int digit0 = mins % 10;
	int digit1 = mins / 10;
	int digit2 = hours % 10;
	int digit3 = hours / 10;

	// single digit hours don't get a leading '0'
	if( !digit3 )
	{
		digit3 = 10;
	}

	// set material texture to use the digits for each number
	SetMaterialIndex( clockLyt, "Clock0", 0, digit0 );
	SetMaterialIndex( clockLyt, "Clock1", 0, digit1 );
	SetMaterialIndex( clockLyt, "Clock2", 0, digit2 );
	SetMaterialIndex( clockLyt, "Clock3", 0, digit3 );
	SetMaterialIndex( clockLyt, "AM_PM_R", 0, pm ? 13 : 12 );// texture 12 is am, 13 is pm


	// theres a brlan that blinks the ':' but i haven't quite figured it out. everything
	// i tried makes it blink too fast.  so for now just use this so it blinks in 2 second cycles
	Pane *pane;
	if( (pane = clockLyt->FindPane( "ClockTen" ) ) )
	{
		pane->SetVisible( !pane->GetVisible() );
	}

	if( ( !timeinfo->tm_hour && !timeinfo->tm_min && !timeinfo->tm_sec ) || !lastTime )
	{
		DateChanged( timeinfo->tm_wday, timeinfo->tm_mday, timeinfo->tm_mon + 1 );
	}

	lastTime = now;
}

bool ChannelGrid::Load( const u8* chanSelAshData, u32 chanSelAshSize )
{
	DeleteEverything();
	loaded = false;
	state = St_Unk;
	//pageNo = 0;
	//lastHighlited = -1;

	U8Archive chanSelArc( chanSelAshData, chanSelAshSize );

	if( !( channelFrame = LoadLayout( chanSelArc, "my_IplTop_a" ) ) )
	{
		DeleteEverything();
		return false;
	}

	if( !( channelStatic = LoadLayout( chanSelArc, "my_IplTop_b" ) ) )
	{
		DeleteEverything();
		return false;
	}

	if( !( clockLyt = LoadLayout( chanSelArc, "my_Clock_a" ) ) )
	{
		DeleteEverything();
		return false;
	}


#define LOADANIM( x )										\
	do														\
	{														\
		if( !(anim = LoadAnimation( chanSelArc, x ) ) )		\
		{													\
			DeleteEverything();								\
			return false;									\
		}													\
		brlans[ x ] = anim;									\
	}														\
	while( 0 )

	// load animations
	Animation *anim;

	LOADANIM( "my_IplTop_a" );
	LOADANIM( "my_IplTop_b" );
	LOADANIM( "my_Clock_a_Change" );						// changes the clock from the initial "Wii Menu" text to a clock

	// create objects
	gridObj = new Object;
	gridObj->BindPane( channelFrame->FindPane( "RootPane" ) );
	gridObj->AddAnimation( brlans.find( "my_IplTop_a" )->second );

	staticObj = new Object;
	staticObj->BindPane( channelStatic->FindPane( "RootPane" ) );
	channelStatic->LoadBrlanTpls( brlans.find( "my_IplTop_b" )->second, chanSelArc );
	staticObj->BindMaterials( channelStatic->Materials() );
	staticObj->AddAnimation( brlans.find( "my_IplTop_b" )->second );
	staticObj->SetAnimation( "my_IplTop_b", 0, 2000 );
	staticObj->Start();

	clockObj = new Object;
	clockObj->BindPane( clockLyt->FindPane( "RootPane" ) );
	clockObj->AddAnimation( brlans.find( "my_Clock_a_Change" )->second );
	clockObj->SetAnimation( "my_Clock_a_Change", 0.f, -1.f, -1.f, false );
	clockObj->Start();

	Pane *pane1;
	SystemMenuButton btn = ChanSel_0;
	u32 x, y, w, h;
	for( int i = 0; i < 12; i++ )
	{
		// since our layouts are drawn and animated directly from the brlyt data,
		// we need to create separate buffers for the brlyts & brlans for each of the highlites

		if( !(highliteBuffer[ i ] = chanSelArc.GetFileAllocated( "/arc/blyt/my_IplTop_d.brlyt" ))
				|| !( highliteBrlanBufferClick[ i ] = chanSelArc.GetFileAllocated( "/arc/anim/my_IplTop_d_Select.brlan" ))
				|| !( highliteBrlanBufferMouseOver[ i ] = chanSelArc.GetFileAllocated( "/arc/anim/my_IplTop_d_FocusOn.brlan" ))
				|| !( highliteBrlanBufferMouseOut[ i ] = chanSelArc.GetFileAllocated( "/arc/anim/my_IplTop_d_FocusOff.brlan" )))
		{
			DeleteEverything();
			return false;
		}

		Layout *l = new Layout;
		if( !l->Load( highliteBuffer[ i ] ) || !l->LoadTextures( chanSelArc ))
		{
			DeleteEverything();
			return false;
		}

		// get button coords
		ButtonCoords( (SystemMenuButton)( btn + i ), x, y, w, h );

		// set position to be drawn
		if( (pane1 = l->FindPane( "RootPane" ) ) )
		{
			f32 x2 = ((((f32)screenwidth) / 2.f) - ( ((f32)screenwidth) - x )) + ( w / 2 );
			f32 y2 = ((((f32)screenheight) / 2.f) - ( ((f32)screenheight) - y )) + ( h / 2 );
			pane1->SetPosition( x2, -y2 );
		}


		// create button
		QuadButton *obj = new QuadButton( x, y, w, h );


		// setup animations
		Animation* anim = new Animation( "my_IplTop_d_Select" );
		anim->Load( (const RLAN_Header *)highliteBrlanBufferClick[ i ] );
		obj->AddAnimation( anim );
		obj->SetMouseOutAnimation( anim, 0, -1 );
		highliteAnims << anim;

		anim = new Animation( "my_IplTop_d_FocusOn" );
		anim->Load( (const RLAN_Header *)highliteBrlanBufferMouseOver[ i ] );
		obj->AddAnimation( anim );
		obj->SetMouseOverAnimation( anim, 0, -1 );
		highliteAnims << anim;

		anim = new Animation( "my_IplTop_d_FocusOff" );
		anim->Load( (const RLAN_Header *)highliteBrlanBufferMouseOut[ i ] );
		obj->AddAnimation( anim );
		obj->SetMouseOutAnimation( anim, 0, -1 );
		highliteAnims << anim;

		// bind stuff
		obj->BindPane( l->FindPane( "RootPane" ) );
		obj->BindMaterials( l->Materials() );

		obj->SetTrigger( Button::Btn_A );

		//leftBtn->Clicked.connect( this, &BannerFrame::LeftButtonClickedSlot );
		//rightBtn->Clicked.connect( this, &BannerFrame::LeftButtonClickedSlot );

		highliteLyts << l;
		channelHighlites << obj;
	}

	// HAXX
	if( (pane1 = channelFrame->FindPane( "N_ChAll" ) ) )
	{
		foreach( Pane *pane, pane1->panes )
		{
			if( !strncmp( pane->getName(), "BaseMask", 8 ) )// 2 is the midle one.  these arent needed anyways
			{
			//	pane->SetVisible( false );
			}
			else if( !strncmp( pane->getName(), "N_Ch_", 5 ) )
			{
				foreach( Pane *chPane, pane->panes )
				{
					iconPanes << chPane;// remomber these bad boys.  they are where the icons bind to
					chPane->SetGxScissorsForchildLayouts( true );
				}
			}
		}
	}
	if( iconPanes.size() != 42 )// there are 42 places for icons in the grid
	{
		gprintf( "didnt find all the panes.\n" );
		return false;
	}
	SetText( clockLyt, "T_WiiMenu", 1 );				// "Wii Menu"

	// theres an AM/PM thing on the right and the left.  remove the left one
	SetPaneVisible( clockLyt, "AM_PM", false );

	// bind the clock to the main grid layout
	if( (pane1 = channelFrame->FindPane( "N_Clock1" ) ) )
	{
		//pane1->SetGxScissorsForchildLayouts( false );
		pane1->BindLayout( clockLyt, false );
	}

	bool widescreen = ( _CONF_GetAspectRatio() > 0 );

	if( widescreen )
	{
		for( int i = 0; i < 5; i++ )
		{
			char name[ 20 ];
			sprintf( name, "Edge%i", i );
			SetMaterialIndex( channelFrame, name, 0, 2 );


			sprintf( name, "Picture_0%i", i );
			SetMaterialIndex( channelFrame, name, 0, 0 );
		}
	}

	channelHighlites[ 0 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked0 );
	channelHighlites[ 1 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked1 );
	channelHighlites[ 2 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked2 );
	channelHighlites[ 3 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked3 );

	channelHighlites[ 4 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked4 );
	channelHighlites[ 5 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked5 );
	channelHighlites[ 6 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked6 );
	channelHighlites[ 7 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked7 );

	channelHighlites[ 8 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked8 );
	channelHighlites[ 9 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked9 );
	channelHighlites[ 10 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked10 );
	channelHighlites[ 11 ]->Clicked.connect( this, &ChannelGrid::ChannelClicked11 );


	gridObj->Finished.connect( this, &ChannelGrid::GridAnimationFinished );


	loaded = true;
	state = St_IdleDown;
	dcState = DSt_None;
	//UpdateClock();
	return true;
}

void ChannelGrid::HidePage( u8 page, bool hide )
{
	char name[ 20 ];
	sprintf( name, "Edge%i", page );

	SetPaneVisible( channelFrame, name, !hide );

	// ?	2	0	1	?
	SetPaneVisible( channelFrame, page == 1 ? "Picture_02" : "Picture_01", !hide );


	sprintf( name, "N_Ch_%c", 'a' + page );
	SetPaneVisible( channelFrame, name, !hide );
}

void ChannelGrid::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded || state == St_IdleUp )
	{
		return;
	}

	HandleUserInput();

	// draw
	//channelFrame->Render( mv, ScreenProps, widescreen );
	channelFrame->Render( modelview, ScreenProps, widescreen );

	// update
	gridObj->Advance();
	staticObj->Advance();
	clockObj->Advance();
	for( u32 i = 0; i < 12; i++ )
	{
		if( state == St_IdleDown )
		{
			highliteLyts[ i ]->Render( modelview, ScreenProps, widescreen );
		}
		channelHighlites[i]->Update();
		channelHighlites[i]->Advance();
	}

	int chIdx = pageNo * 12;
	int sIdx = -15;

	// probably dont need to draw ever banner in the list here.  only the ones on screen
	for( int i = 0; i < 42; i++, sIdx++ )
	{
		Pane *pane = iconPanes[ i ];
		int whichChannelGoesHere = chIdx + iconBindingIdx[ i ];

		if( whichChannelGoesHere >= 0
		   && whichChannelGoesHere < (int)bannerList.size()
		   && bannerList[ whichChannelGoesHere ]->banner
		   && bannerList[ whichChannelGoesHere ]->banner->getIcon())
		{
			if( !bannerList[ whichChannelGoesHere ]->IsBound )
			{
				pane->UnbindAllLayouts();
				pane->BindLayout( bannerList[ whichChannelGoesHere ]->banner->getIcon(), true );
				bannerList[ whichChannelGoesHere ]->IsBound = true;

				if( sIdx >= 0 && sIdx < 12 )
				{
					channelHighlites[ sIdx ]->SetEnabled( true );
				}
			}

			bannerList[ whichChannelGoesHere ]->banner->LoadIcon()->Advance();
		}
	}

	// animate disc channel
	//gprintf( "dcObj->Advance();\n" );
	dcObj->Advance();
	if( dcState != DSt_None )
	{
		//gprintf( "dcIcon->GetInsertDiscObject()->Advance();\n" );
		dcIcon->GetInsertDiscObject()->Advance();
	}

	UpdateClock();

}

void ChannelGrid::DiscInserted( DiHandler::DiscType dt )
{
	switch( dt )
	{
	case DiHandler::T_GC:
		dcLayout = dcIcon->GetGCLayout();
		dcObj = dcIcon->GetGCObject();
		dcState = DSt_GC;
		break;
	case DiHandler::T_Wii:
		dcState = DSt_Wii;
		break;
	case DiHandler::T_Unknown:
		dcLayout = dcIcon->GetNoDiscLayout();
		dcObj = dcIcon->GetNoDiscObject();
		dcState = DSt_None;
		break;
	}

	if( pageNo == 0 )
	{
		iconPanes[ 15 ]->UnbindAllLayouts();
		iconPanes[ 15 ]->BindLayout( dcLayout, true );

		// uberhaxx
		highliteLyts[ 0 ]->FindPane( "RootPane" )->BindLayout( dcIcon->GetInsertDiscLayout(), true );
	}
	dcIcon->StartInsertDiscAnim();
}

void ChannelGrid::DiscEjected()
{
	dcLayout = dcIcon->GetNoDiscLayout();
	dcObj = dcIcon->GetNoDiscObject();

	if( pageNo == 0 )
	{
		iconPanes[ 15 ]->UnbindAllLayouts();
		iconPanes[ 15 ]->BindLayout( dcLayout, true );

		highliteLyts[ 0 ]->FindPane( "RootPane" )->BindLayout( dcIcon->GetInsertDiscLayout(), true );
	}
	dcIcon->StartEjectDiscAnim();
}

void ChannelGrid::SetDiscChannelIcon( Layout *lyt, Object *obj )
{
	if( lyt && obj )
	{
		//gprintf( "ChannelGrid::SetDiscChannelIcon(): setting disc banner animation\n" );
		dcLayout = lyt;
		dcObj = obj;
		dcObj->Start();
	}
	else
	{
		dcLayout = dcIcon->GetNoDiscLayout();
		dcObj = dcIcon->GetNoDiscObject();
	}

	if( pageNo == 0 )
	{
		//gprintf( "ChannelGrid::SetDiscChannelIcon(): binding animation\n" );
		iconPanes[ 15 ]->UnbindAllLayouts();
		iconPanes[ 15 ]->BindLayout( dcLayout, true );

		highliteLyts[ 0 ]->FindPane( "RootPane" )->BindLayout( dcIcon->GetInsertDiscLayout(), true );
	}
}

void ChannelGrid::GridAnimationFinished()
{
	switch( state )
	{
	case St_ShiftUp:
		state = St_IdleUp;
		break;
	case St_ShiftDown:
		state = St_IdleDown;
		break;
	case St_ShiftLeft:
		SetPage( pageNo + 1 );
		state = St_IdleDown;
		break;
	case St_ShiftRight:
		SetPage( pageNo - 1 );
		state = St_IdleDown;
		break;
	default:
		break;
	}
}

void ChannelGrid::HandleUserInput()
{
	if( state == St_IdleDown )
	{
		// check for button presses
		for(int i = 0; i < 4; i++ )
		{
			if( Pad( i ).pPlus() )
			{
				RequestShiftLeft();
				break;
			}
			if( Pad( i ).pMinus() )
			{
				RequestShiftRight();
				break;
			}
		}
	}
}

void ChannelGrid::RequestShiftLeft()
{
	if( state == St_IdleDown && pageNo < pageCnt - 1 )
	{
		// disable highlite buttons
		for( int i = 0; i < 12; i++ )
		{
			channelHighlites[ i ]->SetEnabled( false );
		}

		// start shift animation
		gridObj->SetAnimation( "my_IplTop_a", 40, 60, 0, false );
		gridObj->Start();
		state = St_ShiftLeft;

		if( !pageNo )
		{
			FirstPage( false );
		}
	}
}

void ChannelGrid::RequestShiftRight()
{
	if( state == St_IdleDown && pageNo > 0 )
	{
		// disable highlite buttons
		for( int i = 0; i < 12; i++ )
		{
			channelHighlites[ i ]->SetEnabled( false );
		}

		// start shift animation
		gridObj->SetAnimation( "my_IplTop_a", 0, 20, 0, false );
		gridObj->Start();
		state = St_ShiftRight;

		if( pageNo >= pageCnt - 1 )
		{
			LastPage( false );
		}
	}
}

void ChannelGrid::ChannelClicked0()
{
	ChannelClicked( 0, 0, ( pageNo * 12 ) - 1 );
}

void ChannelGrid::ChannelClicked1()
{
	ChannelClicked( 0, 1, ( pageNo * 12 ) + 0 );
}

void ChannelGrid::ChannelClicked2()
{
	ChannelClicked( 0, 2, ( pageNo * 12 ) + 1 );
}

void ChannelGrid::ChannelClicked3()
{
	ChannelClicked( 0, 3, ( pageNo * 12 ) + 2 );
}

void ChannelGrid::ChannelClicked4()
{
	ChannelClicked( 1, 0, ( pageNo * 12 ) + 3 );
}

void ChannelGrid::ChannelClicked5()
{
	ChannelClicked( 1, 1, ( pageNo * 12 ) + 4 );
}

void ChannelGrid::ChannelClicked6()
{
	ChannelClicked( 1, 2, ( pageNo * 12 ) + 5 );
}

void ChannelGrid::ChannelClicked7()
{
	ChannelClicked( 1, 3, ( pageNo * 12 ) + 6 );
}

void ChannelGrid::ChannelClicked8()
{
	ChannelClicked( 2, 0, ( pageNo * 12 ) + 7 );
}

void ChannelGrid::ChannelClicked9()
{
	ChannelClicked( 2, 1, ( pageNo * 12 ) + 8 );
}

void ChannelGrid::ChannelClicked10()
{
	ChannelClicked( 2, 2, ( pageNo * 12 ) + 9 );
}

void ChannelGrid::ChannelClicked11()
{
	ChannelClicked( 2, 3, ( pageNo * 12 ) + 10 );
}
