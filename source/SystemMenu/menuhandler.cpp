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
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "gecko.h"
#include "menuhandler.h"
#include "object.h"
#include "sc.h"
#include "settings.h"
#include "video.h"

MenuHandler *MenuHandler::instance = NULL;

MenuHandler::MenuHandler()
	: resources( SystemMenuResources::Instance() ),
	  diHandler( DiHandler::Instance() ),
	  state( St_HealthScreen ),
	  selectedBanner( NULL ),
	  bannerSound( NULL ),
	  discChannelBanner( NULL ),
	  discChannelBannerData( NULL ),
	  discChannelBannerDataLen( 0 ),
	  selectedIdx( 0 ),
	  discBannerState( DBSt_None ),
	  buttonPanel( resources->CreateButtonPanel() ),
	  dcIcon( resources->CreateDiscChannelIcon() ),
	  dcBanner( resources->CreateDiscChannel() ),
	  gcBanner( resources->CreateGCBanner() ),
	  homeMenu( NULL ),
	  cursors( resources->Cursors() ),
	  bg( resources->GreyBG() ),
	  grid( resources->CreateChannelGrid() ),
	  setupSelect( NULL ),
	  setupBtn( NULL ),
	  wiiSaveGrid( NULL ),
	  channelEdit( NULL ),
	  bigBannerLayout( NULL ),
	  bigBannerObj( NULL ),
	  homeMenuActive( false ),
      AnimStep( 0 ),
      AnimationRunning( false )
{
	fatarErrorStr[ 0 ] = 0;
	LWP_MutexInit( &drawMutex, false );


}

MenuHandler::~MenuHandler()
{
	delete bg;
	delete buttonPanel;
	delete channelEdit;
	delete cursors;
	delete dcIcon;
	delete dcBanner;
	delete gcBanner;
	delete grid;
	delete homeMenu;
	delete setupBtn;
	delete setupSelect;
	delete wiiSaveGrid;
}

void MenuHandler::BackmenuFinished()
{
	state = St_ChannelGrid;
}

void PrintScreenshot();

void MenuHandler::StartBannerSound()
{
	if( selectedBanner && selectedBanner->LoadSound() )
	{
		bannerSound = new GuiSound( selectedBanner->getSound(), selectedBanner->getSoundSize(), 100 );
		bannerSound->Play();
	}
}

void MenuHandler::StopBannerSound()
{
	if( bannerSound )
	{
		bannerSound->Stop();
		DELETE( bannerSound );
	}
	if( selectedBanner )
	{
		selectedBanner->UnloadSound();
	}
}

void MenuHandler::ChannelIconClicked( u8 row, u8 col, int index )
{
	if( index >= (int)bannerList.size() )
	{
		gprintf( "index >= bannerlist.size()\n" );
		return;
	}

	// TODO: clean this shit up
	// reset animation step for zooming
	AnimStep = 0;
	grid->GetIconPaneCoords( index, &AnimPosX1, &AnimPosY1, &AnimPosX2, &AnimPosY2 );

	selectedIdx = index;
	if( index < 0 )
	{
		switch( discBannerState )
		{
		case DBSt_None:
			dcBanner->Reset();
			state = St_BigBannerFadeIn;
			return;
			break;
		case DBSt_Spinup:
			dcBanner->Reset();
			dcBanner->StartReadingDisc();
			state = St_BigBannerFadeIn;
			return;
			break;
		case DBSt_GCBanner:
			state = St_BigBannerFadeIn;
			return;
			break;
		case DBSt_WiiBanner:
			// load the banner for the inserted disc
			selectedBanner = discChannelBanner;
			break;
		case DBSt_Unknown:
			dcBanner->JumpToUnknownDiscAnim();
			state = St_BigBannerFadeIn;
			return;
			break;
		}
	}
	else
	{
		selectedBanner = bannerList[ index ]->banner;
	}
	if( !selectedBanner )
	{
		gprintf( "!selected banner\n" );
		return;
	}
	if( !( bigBannerObj = selectedBanner->LoadBanner() )
			|| !( bigBannerLayout = selectedBanner->getBanner() ) )
	{
		gprintf( "error loading banner\n" );
		bigBannerObj = NULL;
		bigBannerLayout = NULL;
		selectedBanner = NULL;
		return;
	}


	bigBannerObj->Start();
	state = St_BigBannerFadeIn;
}

void MenuHandler::BannerFrameLeftButtonClicked()
{
	if( state != St_BigBanner )
	{
		gprintf( "state != St_BigBanner\n" );
		return;
	}
	state = St_BigBannerFadeOut;
    AnimStep = 0;
}

void MenuHandler::BannerFrameRightButtonClicked()
{
	if( state != St_BigBanner )
	{
		gprintf( "state != St_BigBanner\n" );
		return;
	}
	state = St_BigBannerFadeOut;
    AnimStep = 0;
}

void MenuHandler::LeftArrowBtnClicked()
{
	switch( state )
	{
	default:
		break;
	case St_ChannelGrid:
	{
		grid->RequestShiftRight();
	}
	break;
	case St_BigBanner:
	{
		StopBannerSound();
		if( selectedBanner )
		{
			selectedBanner->UnloadBanner();
			bigBannerLayout = NULL;
			bigBannerObj = NULL;
			selectedBanner = NULL;
		}

		if( --selectedIdx < -1 )
		{
			selectedIdx = ((int)bannerList.size()) - 1;
		}
		grid->SetPage( (selectedIdx + 1) / 12 );
		grid->GetIconPaneCoords( selectedIdx, &AnimPosX1, &AnimPosY1, &AnimPosX2, &AnimPosY2 );
		if( selectedIdx < 0 )
		{
			switch( discBannerState )
			{
			case DBSt_None:
				dcBanner->Reset();
				return;
				break;
			case DBSt_Spinup:
				dcBanner->Reset();
				dcBanner->StartReadingDisc();
				return;
				break;
			case DBSt_GCBanner:
				return;
				break;
			case DBSt_WiiBanner:
				// load the banner for the inserted disc
				selectedBanner = discChannelBanner;
				break;
			case DBSt_Unknown:
				dcBanner->JumpToUnknownDiscAnim();
				return;
				break;
			}
		}
		else
		{
			selectedBanner = bannerList[ selectedIdx ]->banner;
		}
		if( !selectedBanner )
		{
			gprintf( "!selected banner: %i %u\n", selectedIdx, bannerList.size() );
			state = St_BigBannerFadeOut;
			return;
		}

		// give the async thread a chance to load the banner we want before failing
		int retries = 1000;
		do
		{
			if( ( bigBannerObj = selectedBanner->LoadBanner() ) && ( bigBannerLayout = selectedBanner->getBanner() ) )
			{
				break;
			}
			else if( !retries )
			{
				gprintf( "error loading banner\n" );
				selectedBanner = NULL;
				bigBannerLayout = NULL;
				bigBannerObj = NULL;
				state = St_BigBannerFadeOut;
				return;
			}
			usleep( 1000 );
		}
		while( retries-- );


		bigBannerObj->Start();

		// start playing the sound
		StartBannerSound();
	}
	break;
	}
}

void MenuHandler::RightArrowBtnClicked()
{
	switch( state )
	{
	default:
		break;
	case St_ChannelGrid:
	{
		grid->RequestShiftLeft();
	}
	break;
	case St_BigBanner:
	{
		StopBannerSound();
		if( selectedBanner )
		{
			selectedBanner->UnloadBanner();
			bigBannerLayout = NULL;
			bigBannerObj = NULL;
			selectedBanner = NULL;
		}

		if( ++selectedIdx >= ((int)bannerList.size()) )
		{
			selectedIdx = -1;
		}
		grid->SetPage( (selectedIdx + 1) / 12 );
		grid->GetIconPaneCoords( selectedIdx, &AnimPosX1, &AnimPosY1, &AnimPosX2, &AnimPosY2 );
		if( selectedIdx < 0 )
		{
			switch( discBannerState )
			{
			case DBSt_None:
				dcBanner->Reset();
				return;
				break;
			case DBSt_Spinup:
				dcBanner->Reset();
				dcBanner->StartReadingDisc();
				return;
				break;
			case DBSt_GCBanner:
				return;
				break;
			case DBSt_WiiBanner:
				// load the banner for the inserted disc
				selectedBanner = discChannelBanner;
				break;
			case DBSt_Unknown:
				dcBanner->JumpToUnknownDiscAnim();
				return;
				break;
			}
		}
		else
		{
			selectedBanner = bannerList[ selectedIdx ]->banner;
		}
		if( !selectedBanner )
		{
			gprintf( "!selected banner\n" );
			state = St_BigBannerFadeOut;
			return;
		}
		if( !( bigBannerObj = selectedBanner->LoadBanner() )
				|| !( bigBannerLayout = selectedBanner->getBanner() ) )
		{
			gprintf( "error loading banner\n" );
			selectedBanner = NULL;
			state = St_BigBannerFadeOut;
			return;
		}


		bigBannerObj->Start();

		// start playing the sound
		StartBannerSound();
	}
	break;
	}
}

void MenuHandler::DoGrid()
{

	BuildBannerList();

	// get resources
	LWP_MutexLock( drawMutex );
	BannerFrame *frame = resources->CreateBannerFrame();

	// check for errors
	if( !bg || !grid || !frame || !dcIcon || !dcBanner || !gcBanner )
	{
		FatalError( "Error loading resources for the grid view\n" );
		return;
	}

	if( discChannelBanner )
	{
		if( discBannerState == DBSt_GCBanner )
		{
			grid->DiscInserted( DiHandler::T_GC );
		}
		else if( discBannerState == DBSt_WiiBanner )
		{
			grid->DiscInserted( DiHandler::T_Wii );
		}
		Object *o = discChannelBanner->LoadIcon();
		Layout *l = discChannelBanner->getIcon();
		grid->SetDiscChannelIcon( l, o );
	}

	// connect arrow buttons to the grid
	//buttons->LeftArrow()->Clicked.connect( grid, &ChannelGrid::RequestShiftRight );
	//buttons->RightArrow()->Clicked.connect( grid, &ChannelGrid::RequestShiftLeft );
	//buttonPanel->SettingsBtnClicked.connect( this, &MenuHandler::SettingsBtnClicked );

	grid->FirstPage.connect( buttonPanel, &ButtonPanel::HideLeftArrow );
	grid->LastPage.connect( buttonPanel, &ButtonPanel::HideRightArrow );

	// listen for clicked buttons in the large frame
	frame->RightBtnClicked.connect( this, &MenuHandler::BannerFrameRightButtonClicked );
	frame->LeftBtnClicked.connect( this, &MenuHandler::BannerFrameLeftButtonClicked );

	grid->ChannelListChanged();

	Vec2f ScreenProps;
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;
	bool widescreen = ( _CONF_GetAspectRatio() == 1);

	LWP_MutexUnlock( drawMutex );

	while( state == St_ChannelGrid
		   || state == St_BigBannerFadeIn
		   || state == St_BigBanner
		   || state == St_BigBannerFadeOut )
	{
		//! Update all inputs
		CInputs::Instance()->Update();

		if( Pad( 0 ).pHome() )
		{
			DoHomeMenu();
		}

		if(Pad(0).hPlus() && Pad(0).pMinus())
			PrintScreenshot();

		LWP_MutexLock( drawMutex );
		switch( state )
		{
		default:
			break;
		case St_ChannelGrid:
			buttonPanel->UpdateInput();
			bg->Render( GXmodelView2D, ScreenProps, widescreen );
			grid->Render( GXmodelView2D, ScreenProps, widescreen );
			buttonPanel->Render( GXmodelView2D, ScreenProps, widescreen );
			break;
		case St_BigBannerFadeIn:// TODO: calculate some cool scale matrix up in here
		{
			if( selectedIdx == -1 )
			{
				/*switch( discBannerState )
				{
				case DBSt_None:
					dcBanner->Reset();
					break;
				case DBSt_Spinup:
					dcBanner->Reset();
					dcBanner->StartReadingDisc();
					break;
				case DBSt_GCBanner:
					gprintf( "need the gamecube banner.  exiting...\n" );
					exit( 0 );
					break;
				case DBSt_WiiBanner:// dont do anything
					break;
				case DBSt_Unknown:
					dcBanner->JumpToUnknownDiscAnim();
					break;
				}*/
			}
			else
			{
				if( !bigBannerLayout || !bigBannerObj )
				{
					gprintf( "tried to show a banner with none loaded\n" );
					LWP_MutexUnlock( drawMutex );
					state = St_ChannelGrid;
					continue;
				}
			}

            AnimateZoom(ScreenProps, true);

            // TODO cleanup and optimize the shit
            if(AnimationRunning)
            {
                // load projection for all the usual gui
                GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);

                bg->Render( GXmodelView2D, ScreenProps, widescreen );
                grid->Render( GXmodelView2D, ScreenProps, widescreen );
                buttonPanel->Render( GXmodelView2D, ScreenProps, widescreen );
                // Draw black bg shit
                DrawSquare(0, 0, ScreenProps.x, ScreenProps.y, (GXColor) {0,0,0,BGAlpha});

                // load projection for the banner and frame
                GX_LoadProjectionMtx(BannerProjection, GX_ORTHOGRAPHIC);

                // cut the unneeded crap
                Mtx mv1, mv2, mv3;
                guMtxIdentity (mv2);
                guMtxIdentity (mv3);
                guMtxScaleApply(GXmodelView2D,mv1, 1.f, -1.f, 1.f);
                guMtxTransApply(mv1,mv1, 0.5f * ScreenProps.x, 0.5f * ScreenProps.y, 0.f);
                guMtxTransApply(mv2,mv2, -0.5f * fBannerWidth, 0.5f * fBannerHeight, 0.f);
                guMtxTransApply(mv3,mv3, 0.5f * fBannerWidth, -0.5f * fBannerHeight, 0.f);
                guMtxConcat (mv1, mv2, mv2);
                guMtxConcat (mv1, mv3, mv3);

                f32 viewportv[6];
                f32 projectionv[7];

                GX_GetViewportv(viewportv);
                GX_GetProjectionv(BannerProjection, projectionv, GX_ORTHOGRAPHIC);

                guVector vecTL;
                guVector vecBR;
                GX_Project(0.0f, 0.0f, 0.0f, mv2, projectionv, viewportv, &vecTL.x, &vecTL.y, &vecTL.z);
                GX_Project(0.0f, 0.0f, 0.0f, mv3, projectionv, viewportv, &vecBR.x, &vecBR.y, &vecBR.z);

                // round up scissor box offset and round down the size
                u32 scissorX = (u32)(0.5f + std::max(vecTL.x, 0.0f));
                u32 scissorY = (u32)(0.5f + std::max(vecTL.y, 0.0f));
                u32 scissorW = (u32)std::max(vecBR.x - vecTL.x, 0.0f);
                u32 scissorH = (u32)std::max(vecBR.y - vecTL.y, 0.0f);

                GX_SetScissor( scissorX, scissorY, scissorW, scissorH );

                if(BannerAlpha != 0)
                {
                    if( selectedIdx == -1 )
                    {
                        switch( discBannerState )
                        {
                        case DBSt_Unknown:
                        case DBSt_None:
                        case DBSt_Spinup:
                            //TODO: add banner alpha
                            dcBanner->Render( GXmodelView2D, ScreenProps, widescreen );
                            break;
                        case DBSt_GCBanner:
                            //TODO: add banner alpha
                            gcBanner->Render( GXmodelView2D, ScreenProps, widescreen );
                            break;
                        case DBSt_WiiBanner:
                            bigBannerLayout->Render( GXmodelView2D, ScreenProps, widescreen, BannerAlpha );
                            bigBannerObj->Advance();
                            break;
                        }
                    }
                    else
                    {
                        bigBannerLayout->Render( GXmodelView2D, ScreenProps, widescreen, BannerAlpha );
                        bigBannerObj->Advance();
                    }
                    frame->Render( GXmodelView2D, ScreenProps, widescreen, BannerAlpha );
                }
                // revert scissor and projection
                GX_SetScissor( 0, 0, ScreenProps.x, ScreenProps.y );
                GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);
                break;
            }
            else
            {
                state = St_BigBanner;

                // setup the button panel to go along with the big banner frame
                buttonPanel->ShowArrowsOnly( true );
                buttonPanel->HideLeftArrow( false );
                buttonPanel->HideRightArrow( false );

                grid->FirstPage.disconnect( buttonPanel );
                grid->LastPage.disconnect( buttonPanel );

                // start playing the sound
                StartBannerSound();

                frame->ResetButtons();
                // no break here to render this frame directly with big banner stuff
                // and not skip it and have a black blink
            }
		}
		case St_BigBanner:
		{
			if( state == St_BigBanner )
			{
				// respond to user input
				for( int i = 0; i < 4; i++ )
				{
					if( Pad( i ).pMinus() )
					{
						LeftArrowBtnClicked();
					}
					else if( Pad( i ).pPlus() )
					{
						RightArrowBtnClicked();
					}
					else if( Pad( i ).pB() )
					{
						BannerFrameLeftButtonClicked();
					}
				}
			}

            // cut the unneeded crap
            Mtx mv1, mv2, mv3;
            guMtxIdentity (mv2);
            guMtxIdentity (mv3);
            guMtxScaleApply(GXmodelView2D,mv1, 1.f, -1.f, 1.f);
            guMtxTransApply(mv1,mv1, 0.5f * ScreenProps.x, 0.5f * ScreenProps.y, 0.f);
            guMtxTransApply(mv2,mv2, -0.5f * fBannerWidth, 0.5f * fBannerHeight, 0.f);
            guMtxTransApply(mv3,mv3, 0.5f * fBannerWidth, -0.5f * fBannerHeight, 0.f);
            guMtxConcat (mv1, mv2, mv2);
            guMtxConcat (mv1, mv3, mv3);

            f32 viewportv[6];
            f32 projectionv[7];

            GX_GetViewportv(viewportv);
            GX_GetProjectionv(BannerProjection, projectionv, GX_ORTHOGRAPHIC);

            guVector vecTL;
            guVector vecBR;
            GX_Project(0.0f, 0.0f, 0.0f, mv2, projectionv, viewportv, &vecTL.x, &vecTL.y, &vecTL.z);
            GX_Project(0.0f, 0.0f, 0.0f, mv3, projectionv, viewportv, &vecBR.x, &vecBR.y, &vecBR.z);

            // round up scissor box offset and round down the size
            u32 scissorX = (u32)(0.5f + std::max(vecTL.x, 0.0f));
            u32 scissorY = (u32)(0.5f + std::max(vecTL.y, 0.0f));
            u32 scissorW = (u32)std::max(vecBR.x - vecTL.x, 0.0f);
            u32 scissorH = (u32)std::max(vecBR.y - vecTL.y, 0.0f);

            GX_SetScissor( scissorX, scissorY, scissorW, scissorH );

			if( selectedIdx == -1 )
			{
				switch( discBannerState )
				{
				case DBSt_Unknown:
				case DBSt_None:
				case DBSt_Spinup:
					dcBanner->Render( GXmodelView2D, ScreenProps, widescreen );
					break;
				case DBSt_GCBanner:
					gcBanner->Render( GXmodelView2D, ScreenProps, widescreen );
					break;
				case DBSt_WiiBanner:
					bigBannerLayout->Render( GXmodelView2D, ScreenProps, widescreen );
					bigBannerObj->Advance();
					break;
				}
			}
			else
			{
				bigBannerLayout->Render( GXmodelView2D, ScreenProps, widescreen );
				bigBannerObj->Advance();
			}

			buttonPanel->UpdateInput();
			frame->Render( GXmodelView2D, ScreenProps, widescreen );
			buttonPanel->Render( GXmodelView2D, ScreenProps, widescreen );
		}
		break;
		case St_BigBannerFadeOut:// TODO: render with a scale matrix and zoom back to the grid
		{
            AnimateZoom(ScreenProps, false);

            // TODO cleanup and optimize the shit
            if(AnimationRunning)
            {
                // load projection for all the usual gui
                GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);

                bg->Render( GXmodelView2D, ScreenProps, widescreen );
                grid->Render( GXmodelView2D, ScreenProps, widescreen );
                buttonPanel->Render( GXmodelView2D, ScreenProps, widescreen );
                // Draw black bg shit
                DrawSquare(0, 0, ScreenProps.x, ScreenProps.y, (GXColor) {0,0,0,BGAlpha});

                // load projection for the banner and frame
                GX_LoadProjectionMtx(BannerProjection, GX_ORTHOGRAPHIC);

                // cut the unneeded crap
                Mtx mv1, mv2, mv3;
                guMtxIdentity (mv2);
                guMtxIdentity (mv3);
                guMtxScaleApply(GXmodelView2D,mv1, 1.f, -1.f, 1.f);
                guMtxTransApply(mv1,mv1, 0.5f * ScreenProps.x, 0.5f * ScreenProps.y, 0.f);
                guMtxTransApply(mv2,mv2, -0.5f * fBannerWidth, 0.5f * fBannerHeight, 0.f);
                guMtxTransApply(mv3,mv3, 0.5f * fBannerWidth, -0.5f * fBannerHeight, 0.f);
                guMtxConcat (mv1, mv2, mv2);
                guMtxConcat (mv1, mv3, mv3);

                f32 viewportv[6];
                f32 projectionv[7];

                GX_GetViewportv(viewportv);
                GX_GetProjectionv(BannerProjection, projectionv, GX_ORTHOGRAPHIC);

                guVector vecTL;
                guVector vecBR;
                GX_Project(0.0f, 0.0f, 0.0f, mv2, projectionv, viewportv, &vecTL.x, &vecTL.y, &vecTL.z);
                GX_Project(0.0f, 0.0f, 0.0f, mv3, projectionv, viewportv, &vecBR.x, &vecBR.y, &vecBR.z);

                // round up scissor box offset and round down the size
                u32 scissorX = (u32)(0.5f + std::max(vecTL.x, 0.0f));
                u32 scissorY = (u32)(0.5f + std::max(vecTL.y, 0.0f));
                u32 scissorW = (u32)std::max(vecBR.x - vecTL.x, 0.0f);
                u32 scissorH = (u32)std::max(vecBR.y - vecTL.y, 0.0f);

                GX_SetScissor( scissorX, scissorY, scissorW, scissorH );

                if(BannerAlpha != 0)
                {
                    if( selectedIdx == -1 )
                    {
                        switch( discBannerState )
                        {
                        case DBSt_Unknown:
                        case DBSt_None:
                        case DBSt_Spinup:
                            //TODO: add banner alpha
                            dcBanner->Render( GXmodelView2D, ScreenProps, widescreen );
                            break;
                        case DBSt_GCBanner:
                            //TODO: add banner alpha
                            gcBanner->Render( GXmodelView2D, ScreenProps, widescreen );
                            break;
                        case DBSt_WiiBanner:
                            bigBannerLayout->Render( GXmodelView2D, ScreenProps, widescreen, BannerAlpha );
                            bigBannerObj->Advance();
                            break;
                        }
                    }
                    else
                    {
                        bigBannerLayout->Render( GXmodelView2D, ScreenProps, widescreen, BannerAlpha );
                        bigBannerObj->Advance();
                    }
                    frame->Render( GXmodelView2D, ScreenProps, widescreen, BannerAlpha );
                }
                // revert scissor and projection
                GX_SetScissor( 0, 0, ScreenProps.x, ScreenProps.y );
                GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);
            }
            else
            {
                state = St_ChannelGrid;

                // setup the button panel again for the grid mode
                buttonPanel->ShowArrowsOnly( false );

                grid->FirstPage.connect( buttonPanel, &ButtonPanel::HideLeftArrow );
                grid->LastPage.connect( buttonPanel, &ButtonPanel::HideRightArrow );

                grid->SetPage( ( selectedIdx + 1 ) / 12 );
                StopBannerSound();
                if( selectedBanner )
                {
                    selectedBanner->UnloadBanner();
                    bigBannerLayout = NULL;
                    bigBannerObj = NULL;
                    selectedBanner = NULL;
                }
                // do a new cycle to render this frame and not leave it blank blink
                LWP_MutexUnlock( drawMutex );
                continue;
            }
		}
        break;
		}

		//dcIcon->Render( GXmodelView2D, ScreenProps, widescreen );
		cursors->Render( GXmodelView2D, ScreenProps, widescreen );


		LWP_MutexUnlock( drawMutex );
		Menu_Render();
	}

	grid->FirstPage.disconnect( buttonPanel );
	grid->LastPage.disconnect( buttonPanel );

	delete frame;

	// done playing banner sound
	StopBannerSound();

	// free the buffer containing the decompressed ash data for the big frame
	resources->DestroyBannerFrame();

	// make sure the grid doesnt still contain pointers to banners, since we might be going to the settings to delete or add a channel
	grid->UnbindAllChannels();
	selectedBanner = NULL;

	// clear the bannerlist
	FreeBannerList();
}

void MenuHandler::SettingsBtnClicked()
{
	state = St_SettingsSelect;
}

void MenuHandler::DoHealthScreen()
{
	HealthScreen *health = resources->CreateHealthScreen();
	if( !health )
	{
		FatalError( "Error creating health screen\n" );
		return;
	}

	health->Done.connect( this, &MenuHandler::BackmenuFinished );

	Vec2f ScreenProps;
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;
	bool widescreen = ( _CONF_GetAspectRatio() == 1);

	while( state == St_HealthScreen )
	{
		if(Pad(0).hPlus() && Pad(0).pMinus())
			PrintScreenshot();

		//! Update all inputs
		CInputs::Instance()->Update();

		health->Render( GXmodelView2D, ScreenProps, widescreen );
		Menu_Render();
	}

	resources->DestroyHealthScreen();
}

void MenuHandler::ShowWiiSaveMenu( bool show )
{
	if( show )
	{
		BuildSaveList( BannerBin::Nand );
		state = St_WiiSaves;
	}
	else
	{
		FreeSaveList();
		state = St_SettingsSelect;
	}
}

void MenuHandler::DoSettingsSelect()
{
	SettingsBG *bg = resources->CreateSettingsBG();
	setupBtn = resources->CreateSettingsBtn();
	setupSelect = resources->CreateSettingsSelect();
	wiiSaveGrid = resources->CreateSaveGrid();
	channelEdit = resources->CreateChannelEdit();
	DialogWindow *yesNoDlg = resources->CreateDialog( DialogWindow::B_2 );


	if( !bg || !setupBtn || !setupSelect || !wiiSaveGrid || !yesNoDlg || !channelEdit )
	{
		FatalError( "Error creating settings menus" );
		return;
	}

	wiiSaveGrid->SetDialog( yesNoDlg );
	wiiSaveGrid->SetHidden( true );
	channelEdit->SetDialog( yesNoDlg );
	channelEdit->SetHidden( true );

	// connect back button to the select resource
	setupBtn->BackClicked.connect( setupSelect, &SettingsSelect::BackBtnClicked );
	setupBtn->BackClicked.connect( wiiSaveGrid, &SaveGrid::BackButtonClicked );
	setupBtn->BackClicked.connect( channelEdit, &ChannelEdit::BackButtonClicked );

	setupSelect->HideWii.connect( setupBtn, &SettingsBtn::HideWiiLogo );

	setupSelect->AppendWiiSaveData.connect( this, &MenuHandler::EnterLeaveWiiSaveData );
	setupSelect->AppendChannelManager.connect( this, &MenuHandler::EnterLeaveChannelManager );
	setupSelect->ExitSettings.connect( this, &MenuHandler::LeaveSettings );
	wiiSaveGrid->DisableBackButton.connect( setupBtn, &SettingsBtn::DisableBackBtn );
	channelEdit->DisableBackButton.connect( setupBtn, &SettingsBtn::DisableBackBtn );

	wiiSaveGrid->Done.connect( setupSelect, &SettingsSelect::WiiSaveDone );
	channelEdit->Done.connect( setupSelect, &SettingsSelect::ChannelEditDone );

	Vec2f ScreenProps;
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;
	bool widescreen = ( _CONF_GetAspectRatio() == 1);

	while( state == St_SettingsSelect || state == St_WiiSaves || state == St_ChannEdit )
	{
		if(Pad(0).hPlus() && Pad(0).pMinus())
			PrintScreenshot();

		//! Update all inputs
		CInputs::Instance()->Update();

		if( Pad( 0 ).pHome() )// for quick testing
		{
			//state = St_Exit;
			//break;
			DoHomeMenu();
		}
		switch( state )
		{
		case St_SettingsSelect:
			bg->Render( GXmodelView2D, ScreenProps, widescreen );
			setupBtn->Render( GXmodelView2D, ScreenProps, widescreen );
			setupSelect->Render( GXmodelView2D, ScreenProps, widescreen );
			break;
		case St_WiiSaves:
			setupBtn->Render( GXmodelView2D, ScreenProps, widescreen );
			setupSelect->Render( GXmodelView2D, ScreenProps, widescreen );
			wiiSaveGrid->Render( GXmodelView2D, ScreenProps, widescreen );
			break;
		case St_ChannEdit:
			setupBtn->Render( GXmodelView2D, ScreenProps, widescreen );
			setupSelect->Render( GXmodelView2D, ScreenProps, widescreen );
			channelEdit->Render( GXmodelView2D, ScreenProps, widescreen );
			break;
		default:
			//gprintf( "don\'t do that\n" );
			//exit( 0 );
			break;
		}

		cursors->Render( GXmodelView2D, ScreenProps, widescreen );
		Menu_Render();
	}

	DELETE( setupSelect );
	DELETE( setupBtn );
	DELETE( wiiSaveGrid );
	DELETE( channelEdit );
	delete bg;
	delete yesNoDlg;

	resources->DestroyChannelEdit();
	resources->DestroySaveGrid();
	resources->DestroySettingsBtn();
	resources->DestroySettingsSelect();
	resources->DestroySettingsBG();
}

void MenuHandler::DoHomeMenu()
{
	homeMenuActive = true;

	homeMenu = resources->CreateHomeMenu();
	if( !homeMenu )
	{
		FatalError( "Error creating home menu\n" );
		return;
	}
	/*u32 texBufLen;
	u16 texWidth;
	u16 texHeight;
	u8* texBuf = CreateTextureFromFrameBuffer( GX_TF_RGB565, texBufLen, texWidth, texHeight );
	Texture *bgTex = NULL;
	if( texBuf )
	{
		bgTex = new Texture;
		bgTex->LoadFromRawData( texBuf, texWidth, texHeight, GX_TF_RGB565 );
		homeMenu->SetBGTexture( bgTex );

		hexdump( texBuf, 0x800 );
	}*/

	homeMenu->Done.connect( this, &MenuHandler::HomeMenuCancel );
	homeMenu->ExitToWiiMenu.connect( this, &MenuHandler::HomeMenuWantsToExit );

	Vec2f ScreenProps;
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;
	bool widescreen = ( _CONF_GetAspectRatio() == 1);

	while( homeMenuActive )
	{
		if(Pad(0).hPlus() && Pad(0).pMinus())
			PrintScreenshot();

		//! Update all inputs
		CInputs::Instance()->Update();

		homeMenu->Render( GXmodelView2D, ScreenProps, widescreen );
		cursors->Render( GXmodelView2D, ScreenProps, widescreen );
		Menu_Render();
	}
	delete homeMenu;
	resources->DestroyHomeMenu();

	//delete bgTex;
	//free( texBuf );
}

void MenuHandler::HomeMenuWantsToExit()
{
	homeMenuActive = false;
	state = St_Exit;
}

void MenuHandler::HomeMenuCancel()
{
	homeMenuActive = false;
}

void MenuHandler::EnterLeaveWiiSaveData( bool enter )
{
	if( enter )
	{
		if( state != St_SettingsSelect )
		{
			gprintf( "state != St_SettingsSelect\n" );
			return;
		}
		state = St_WiiSaves;
		BuildSaveList( BannerBin::Nand );
		wiiSaveGrid->SetPage( 0 );
	}
	else
	{
		if( state != St_WiiSaves )
		{
			gprintf( "state != St_WiiSaves\n" );
			return;
		}
		state = St_SettingsSelect;
		FreeSaveList();
		wiiSaveGrid->SetPage( 0 );
	}
	wiiSaveGrid->SetHidden( !enter );
}

void MenuHandler::EnterLeaveChannelManager( bool enter )
{
	if( enter )
	{
		if( state != St_SettingsSelect )
		{
			gprintf( "state != St_SettingsSelect\n" );
			return;
		}
		state = St_ChannEdit;
		FreeBannerList();
		BuildBannerList( NandUserChannels );
		channelEdit->SetPage( 0 );
	}
	else
	{
		if( state != St_ChannEdit )
		{
			gprintf( "state != St_ChannEdit\n" );
			return;
		}
		state = St_SettingsSelect;
		FreeBannerList();
		channelEdit->SetPage( 0 );
	}
	channelEdit->SetHidden( !enter );
}

void MenuHandler::LeaveSettings()
{
	state = St_ChannelGrid;
}

void MenuHandler::DoFatalError()
{
	FatalErrordialog *dlg = resources->FatarError();
	dlg->SetMessage( fatarErrorStr );

	Vec2f ScreenProps;
	ScreenProps.x = screenwidth;
	ScreenProps.y = screenheight;
	bool widescreen = ( _CONF_GetAspectRatio() == 1);

	while( state == St_FatalError )
	{
		if(Pad(0).hPlus() && Pad(0).pMinus())
			PrintScreenshot();

		//! Update all inputs
		CInputs::Instance()->Update();

		for( int i = 0; i < 4; i++ )
		{
			if( Pad( i ).pHome() || Pad( i ).pA() )
			{
				state = St_Exit;
				break;
			}
		}

		dlg->Render( GXmodelView2D, ScreenProps, widescreen );
		Menu_Render();
	}

}

void MenuHandler::FatalError( const char* fmt, ... )
{
	state = St_FatalError;

	char *charStr = NULL;

	va_list argptr;
	va_start( argptr, fmt );
	int ret = vasprintf( &charStr, fmt, argptr );
	va_end( argptr );

	// some error
	if( ret < 1 || !charStr )
	{
		strcpy16( fatarErrorStr, "A Fatal Error occurred while\nsetting the fatal error message.\n" );
		return;
	}
	strlcpy16( fatarErrorStr, charStr, FatarErrorStrLen );
	free( charStr );
}

void MenuHandler::Start()
{
	//state = St_ChannelGrid;// speed up testing.  the health screen already works
	//state = St_SettingsSelect;

	if ( Settings::mountDVD )
	{
		diHandler->Wake();
	}

	// make sure the serources were created right
	if( !cursors )
	{
		FatalError( "Error creating cursors\n" );
	}
	else if( !buttonPanel )
	{
		FatalError( "Error creating button panel\n" );
	}
	else if( !grid )
	{
		FatalError( "Error creating grid\n" );
	}
	else if( !dcBanner )
	{
		FatalError( "Error creating dcBanner\n" );
	}
	else if( !dcIcon )
	{
		FatalError( "Error creating dcIcon\n" );
	}
	else
	{
		// everything looks like it was created, connect the resources together

		// connect the DiHandler to this
		diHandler->DiscInserted.connect( this, &MenuHandler::DiscInserted );
		diHandler->DiscEjected.connect( this, &MenuHandler::DiscEjected );
		diHandler->OpeningBnrReady.connect( this, &MenuHandler::GetDiscChannelBannerData );
		diHandler->StartingToReadDisc.connect( this, &MenuHandler::StartingToReadDisc );

		// connect the clock to the date text
		grid->DateChanged.connect( bg, &GreyBackground::SetDate );

		// so we know when a channel was clicked
		grid->ChannelClicked.connect( this, &MenuHandler::ChannelIconClicked );

		// so we know when the disc channel "insert disc" animation is done
		dcBanner->Finished.connect( this, &MenuHandler::DiscChannelAnimFinished );

		// let the grid know where the disc channel icon is
		grid->SetDiscChannelPointer( dcIcon );

		// listen for right/left arrow clicks
		buttonPanel->LeftArrow()->Clicked.connect( this, &MenuHandler::LeftArrowBtnClicked );
		buttonPanel->RightArrow()->Clicked.connect( this, &MenuHandler::RightArrowBtnClicked );
		buttonPanel->SettingsBtnClicked.connect( this, &MenuHandler::SettingsBtnClicked );
	}


	while( state != St_Exit )
	{
		switch( state )
		{
		case St_FatalError:
			DoFatalError();
			break;
		case St_HealthScreen:
			DoHealthScreen();
			break;
		case St_ChannelGrid:
			DoGrid();
			break;
		case St_SettingsSelect:
			DoSettingsSelect();
			break;
		default:
			break;
		}
	}
}

void MenuHandler::DiscInserted( DiHandler::DiscType dt )
{
	LWP_MutexLock( drawMutex );
	if( grid && dt != DiHandler::T_Wii )
	{
		grid->DiscInserted( dt );
	}
	if( selectedIdx == -1 && ( state == St_BigBannerFadeIn || state == St_BigBanner ) )
	{
		switch( dt )
		{
		case DiHandler::T_GC:
			dcBanner->DiscIsGC();
			break;
		case DiHandler::T_Wii:
			dcBanner->DiscIsWii();
			break;
		case DiHandler::T_Unknown:
			discBannerState = DBSt_Unknown;
			dcBanner->DiscIsUnknown();
			break;
		}
	}
	else
	{
		switch( dt )
		{
		case DiHandler::T_GC:
			discBannerState = DBSt_GCBanner;
			break;
		case DiHandler::T_Wii:
			discBannerState = DBSt_WiiBanner;
			break;
		case DiHandler::T_Unknown:
			discBannerState = DBSt_Unknown;
			break;
		}

	}
	LWP_MutexUnlock( drawMutex );
}

void MenuHandler::DiscEjected()
{
	LWP_MutexLock( drawMutex );

	if( selectedIdx == -1 && state == St_BigBanner )
	{
		StopBannerSound();
	}

	if( grid )
	{
		grid->DiscEjected();
	}
	if( dcBanner )
	{
		if( selectedIdx == -1 && ( state == St_BigBannerFadeIn || state == St_BigBanner ) )// the disc channel was showing the banner before
		{
			selectedBanner = NULL;
			if( discBannerState == DBSt_Spinup )
			{
				dcBanner->EjectDisc();
			}
			else
			{
				dcBanner->Reset();
			}
		}
	}

	DELETE( discChannelBanner );
	FREE( discChannelBannerData );

	discBannerState = DBSt_None;

	LWP_MutexUnlock( drawMutex );
}

void MenuHandler::GetDiscChannelBannerData( u8* data, u32 len, bool &taken )
{
	LWP_MutexLock( drawMutex );

	if( data )
	{
		// create banner instance
		discChannelBannerData = data;
		discChannelBannerDataLen = len;
		discChannelBanner = new Banner( discChannelBannerData, discChannelBannerDataLen );

		// load icon for small disc channel
		Object *o = discChannelBanner->LoadIcon();
		Layout *l = discChannelBanner->getIcon();
		if( grid )
		{
			grid->SetDiscChannelIcon( l, o );
			grid->DiscInserted( DiHandler::T_Wii );
		}

		// if we are currently looking at the disc channel, load the big banner up
		if( selectedIdx == -1 && ( state == St_BigBannerFadeIn || state == St_BigBanner ) )
		{
			selectedBanner = discChannelBanner;
			if( !( bigBannerObj = selectedBanner->LoadBanner() )
				|| !( bigBannerLayout = selectedBanner->getBanner() ) )
			{
				gprintf( "error loading banner for disc channel\n" );
				bigBannerObj = NULL;
				bigBannerLayout = NULL;
				selectedBanner = NULL;
				return;
			}


		}
		if( state == St_BigBannerFadeIn || state == St_BigBanner )
		{
			// tell the disc channel to play the "insert wii disc" animation
			dcBanner->DiscIsWii();
		}
		else
		{
			discBannerState = DBSt_WiiBanner;
		}
		taken = true;
	}
	LWP_MutexUnlock( drawMutex );
}

void MenuHandler::StartingToReadDisc()
{
	LWP_MutexLock( drawMutex );
	discBannerState = DBSt_Spinup;
	if( dcBanner && ( state == St_BigBannerFadeIn || state == St_BigBanner ) )
	{
		dcBanner->StartReadingDisc();
	}
	LWP_MutexUnlock( drawMutex );
}

void MenuHandler::DiscChannelAnimFinished()
{
	if( discBannerState == DBSt_Spinup && discChannelBanner )
	{
		discBannerState = DBSt_WiiBanner;
		if( selectedIdx == -1 && state == St_BigBanner )
		{
			StartBannerSound();
		}
	}
	else
	{
		discBannerState = DBSt_GCBanner;
	}
}

void MenuHandler::AnimateZoom(const Vec2f &ScreenProps, bool AnimZoomIn)
{
    static const bool PAL50 = (CONF_GetVideo() == CONF_VIDEO_PAL) && (CONF_GetEuRGB60() == 0);
    float xDiff = 310.f;
    float yDiff = 236.0f;
    if(PAL50)
        yDiff = 220.0f;

    // animation is on going
	if(AnimStep < MaxAnimSteps)
	{
        AnimationRunning = true;
        AnimStep++;

        // zoom in animation
        if(AnimZoomIn) {
            BGAlpha = std::min(255.f * AnimStep * 2.f / MaxAnimSteps, 255.f);
            if(AnimStep < 0.4f * MaxAnimSteps)
                BannerAlpha = 0;
            else
                BannerAlpha = std::min(255.f * (AnimStep - 0.4f * MaxAnimSteps) / (0.6f * MaxAnimSteps), 255.f);
        }
        // zoom out animation
        else {
            BGAlpha = std::min(255.f * (MaxAnimSteps-AnimStep) * 2.f / MaxAnimSteps, 255.f);
            if((MaxAnimSteps - AnimStep) < 0.4f * MaxAnimSteps)
                BannerAlpha = 0;
            else
                BannerAlpha = std::min(255.f * ((MaxAnimSteps - AnimStep) - 0.4f * MaxAnimSteps) / (0.6f * MaxAnimSteps), 255.f);
        }

        //! This works good for banners
        f32 chopXOffset = (ScreenProps.x * 0.5f - xDiff );
        f32 chopYOffset = (ScreenProps.y * 0.5f - yDiff );
        f32 chopX = (ScreenProps.x * 0.5f + xDiff );
        f32 chopY = (ScreenProps.y * 0.5f + yDiff );


        float curAnimStep = AnimZoomIn ? ((float)(MaxAnimSteps - AnimStep)/(float)MaxAnimSteps) : ((float)AnimStep/(float)MaxAnimSteps);

		float stepx1 = chopXOffset - AnimPosX1;
		float stepy1 = chopYOffset - AnimPosY1;
        float stepx2 = chopX - AnimPosX2;
		float stepy2 = chopY - AnimPosY2;

		float top = AnimPosY1 + stepy1 * curAnimStep;
		float bottom = AnimPosY2 + stepy2 * curAnimStep;
		float left = AnimPosX1 + stepx1 * curAnimStep;
		float right = AnimPosX2 + stepx2 * curAnimStep;

        // set main projection of all GUI stuff
        guOrtho(MainProjection, top, bottom, left, right, -1000, 1000);

        // this just looks better for banner/icon ratio
        f32 ratioX = xDiff * 2.f / 128.f;
        f32 ratioY = yDiff * 2.f / 96.f;

        stepx1 = ((ScreenProps.x * 0.5f - xDiff) - AnimPosX1) * ratioX;
        stepx2 = ((ScreenProps.x * 0.5f + xDiff) - AnimPosX2) * ratioX;
        stepy1 = ((ScreenProps.y * 0.5f - yDiff) - AnimPosY1) * ratioY;
        stepy2 = ((ScreenProps.y * 0.5f + yDiff) - AnimPosY2) * ratioY;

        //! This works good for banners
		top = (ScreenProps.y * 0.5f - yDiff) + stepy1 * curAnimStep;
        bottom = (ScreenProps.y * 0.5f + yDiff) + stepy2 * curAnimStep;
        left = (ScreenProps.x * 0.5f - xDiff) + stepx1 * curAnimStep;
		right = (ScreenProps.x * 0.5f + xDiff) + stepx2 * curAnimStep;

        // set banner projection
		guOrtho(BannerProjection,top, bottom, left, right,-1000,1000);
	}
    else {
        //! This works good for banners
        f32 chopXOffset = (ScreenProps.x * 0.5f - xDiff );
        f32 chopYOffset = (ScreenProps.y * 0.5f - yDiff );
        f32 chopX = (ScreenProps.x * 0.5f + xDiff );
        f32 chopY = (ScreenProps.y * 0.5f + yDiff );

        guOrtho(MainProjection,chopYOffset,chopY,chopXOffset,chopX,-1000,1000);
        GX_LoadProjectionMtx(MainProjection, GX_ORTHOGRAPHIC);
        AnimationRunning = false;
    }
}
