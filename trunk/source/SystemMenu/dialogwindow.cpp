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
#include "dialogwindow.h"


DialogWindow::DialogWindow()
	: layoutA0( NULL ),
	  objA0( NULL ),
	  objA0prog( NULL ),
	  objA0wait( NULL ),

	  layoutA1( NULL ),
	  objA1( NULL ),
	  objA1btn1( NULL ),

	  layoutA2( NULL ),
	  objA2( NULL ),
	  objA2btn1( NULL ),
	  objA2btn2( NULL ),

	  layoutA3( NULL ),
	  objA3( NULL ),
	  objA3btn1( NULL ),
	  objA3btn2( NULL ),
	  objA3btn3( NULL ),

	  layoutB( NULL ),
	  objB( NULL ),
	  objBbtn1( NULL ),
	  objBbtn2( NULL )
{
}
DialogWindow::~DialogWindow()
{
	delete objA0;
	delete objA0prog;
	delete objA0wait;

	delete objA1;
	delete objA1btn1;

	delete objA2;
	delete objA2btn1;
	delete objA2btn2;

	delete objA3;
	delete objA3btn1;
	delete objA3btn2;
	delete objA3btn3;

	delete objB;
	delete objBbtn1;
	delete objBbtn2;

	delete layoutA0;
	delete layoutA1;
	delete layoutA2;
	delete layoutA3;
	delete layoutB;

	std::map< std::string, Animation *>::iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

bool DialogWindow::Load( const u8* dlgWdwAsh, u32 dlgWdwAshSize, Type type)
{
	if( loaded )
	{
		return true;
	}
	if( !dlgWdwAsh || !dlgWdwAshSize )
	{
		return false;
	}
	u32 x, y, w, h;

	U8Archive dlgWdwArc( dlgWdwAsh, dlgWdwAshSize );

#define LOADANIM( x, y, z )									\
	do														\
	{														\
		if( !(anim = LoadAnimation( dlgWdwArc, x ) ) )		\
		{													\
			return false;									\
		}													\
		brlans[ x ] = anim;									\
		y->AddAnimation( anim );							\
		z->LoadBrlanTpls( anim, dlgWdwArc );				\
	}														\
	while( 0 )
	Animation *anim;


	switch( type )
	{
	case A0:
	{
		// create dialog A0
		if( !( layoutA0 = LoadLayout( dlgWdwArc, "my_DialogWindow_a0" ) ) )
		{
			return false;
		}

		objA0 = new Object;
		objA0prog = new Object;
		objA0wait = new Object;

		LOADANIM( "my_DialogWindow_a0_DialogIn", objA0, layoutA0 );			// window slides in from the bottom
		LOADANIM( "my_DialogWindow_a0_DialogOut", objA0, layoutA0 );		// window slides off the top of the page
		LOADANIM( "my_DialogWindow_a0_Progres", objA0prog, layoutA0 );			// used to fill up the progress bar
		LOADANIM( "my_DialogWindow_a0_Wait", objA0wait, layoutA0 );

		objA0->BindGroup( layoutA0->FindGroup( "G_InOut" ));
		objA0->SetAnimation( "my_DialogWindow_a0_DialogIn", 0, -1, -1, false );

		objA0prog->BindGroup( layoutA0->FindGroup( "G_Prog" ));
		objA0prog->SetAnimation( "my_DialogWindow_a0_Progres" );

		objA0wait->BindGroup( layoutA0->FindGroup( "G_Wait" ));
		objA0wait->SetAnimation( "my_DialogWindow_a0_Wait" );
		objA0wait->BindMaterial( layoutA0->FindMaterial( "Wait_00" ) );

		objA0->Start();
		objA0wait->Start();

		objA0->Finished.connect( this, &DialogWindow::WindowAnimDone );
	}
	break;
	case A1:
	{
		// create dialog A1
		if( !( layoutA1 = LoadLayout( dlgWdwArc, "my_DialogWindow_a1" ) ) )
		{
			return false;
		}

		ButtonCoords( Dlg_A1_Btn1, x, y, w, h );
		objA1 = new Object;
		objA1btn1 = new QuadButton( x, y, w, h );

		LOADANIM( "my_DialogWindow_a1_DialogIn", objA1, layoutA1 );				// window slides in from the bottom
		LOADANIM( "my_DialogWindow_a1_DialogOut", objA1, layoutA1 );			// window slides off the top of the page
		LOADANIM( "my_DialogWindow_a1_FocusBtn_off", objA1btn1, layoutA1 );		// button over/out/click
		LOADANIM( "my_DialogWindow_a1_FocusBtn_on", objA1btn1, layoutA1 );
		LOADANIM( "my_DialogWindow_a1_SelectBtn_Ac", objA1btn1, layoutA1 );

		objA1->BindGroup( layoutA1->FindGroup( "G_InOut" ) );
		objA1->SetAnimation( "my_DialogWindow_a1_DialogIn", 0, -1, -1, false );

		objA1btn1->BindGroup( layoutA1->FindGroup( "G_SelectBtnB" ) );
		objA1btn1->SetMouseOverAnimation( brlans.find( "my_DialogWindow_a1_FocusBtn_on" )->second, 0, -1 );
		objA1btn1->SetMouseOutAnimation( brlans.find( "my_DialogWindow_a1_FocusBtn_off" )->second, 0, -1 );
		objA1btn1->SetClickAnimation( brlans.find( "my_DialogWindow_a1_SelectBtn_Ac" )->second, 0, -1 );
		objA1btn1->SetTrigger( Button::Btn_A );

		objA1btn1->Clicked.connect( this, &DialogWindow::Button1Clicked );


		objA1->Start();
		objA1btn1->Start();

		objA1->Finished.connect( this, &DialogWindow::WindowAnimDone );
	}
	break;
	case A2:
	{
		// create dialog A2
		if( !( layoutA2 = LoadLayout( dlgWdwArc, "my_DialogWindow_a2" ) ) )
		{
			return false;
		}

		ButtonCoords( Dlg_A2_Btn1, x, y, w, h );
		objA2 = new Object;
		objA2btn1 = new QuadButton( x, y, w, h );
		ButtonCoords( Dlg_A2_Btn2, x, y, w, h );
		objA2btn2 = new QuadButton( x, y, w, h );

		LOADANIM( "my_DialogWindow_a2_DialogIn", objA2, layoutA2 );				// window slides in from the bottom
		LOADANIM( "my_DialogWindow_a2_DialogOut", objA2, layoutA2 );			// window slides off the top of the page
		LOADANIM( "my_DialogWindow_a2_FocusBtn_off", objA2btn1, layoutA2 );		// button over/out/click
		LOADANIM( "my_DialogWindow_a2_FocusBtn_on", objA2btn1, layoutA2 );
		LOADANIM( "my_DialogWindow_a2_SelectBtn_Ac", objA2btn1, layoutA2 );

		objA2->BindGroup( layoutA2->FindGroup( "G_InOut" ) );
		objA2->SetAnimation( "my_DialogWindow_a2_DialogIn", 0, -1, -1, false );

		objA2btn1->BindGroup( layoutA2->FindGroup( "G_SelectBtnA" ) );
		objA2btn1->SetMouseOverAnimation( brlans.find( "my_DialogWindow_a2_FocusBtn_on" )->second, 0, -1 );
		objA2btn1->SetMouseOutAnimation( brlans.find( "my_DialogWindow_a2_FocusBtn_off" )->second, 0, -1 );
		objA2btn1->SetClickAnimation( brlans.find( "my_DialogWindow_a2_SelectBtn_Ac" )->second, 0, -1 );
		objA2btn1->SetTrigger( Button::Btn_A );

		objA2btn2->AddAnimation( brlans.find( "my_DialogWindow_a2_FocusBtn_on" )->second );
		objA2btn2->AddAnimation( brlans.find( "my_DialogWindow_a2_FocusBtn_off" )->second );
		objA2btn2->AddAnimation( brlans.find( "my_DialogWindow_a2_SelectBtn_Ac" )->second );
		objA2btn2->BindGroup( layoutA2->FindGroup( "G_SelectBtnB" ) );
		objA2btn2->SetMouseOverAnimation( brlans.find( "my_DialogWindow_a2_FocusBtn_on" )->second, 0, -1 );
		objA2btn2->SetMouseOutAnimation( brlans.find( "my_DialogWindow_a2_FocusBtn_off" )->second, 0, -1 );
		objA2btn2->SetClickAnimation( brlans.find( "my_DialogWindow_a2_SelectBtn_Ac" )->second, 0, -1 );
		objA2btn2->SetTrigger( Button::Btn_A );

		objA2btn1->Clicked.connect( this, &DialogWindow::Button1Clicked );
		objA2btn2->Clicked.connect( this, &DialogWindow::Button2Clicked );


		objA2->Start();
		objA2btn1->Start();
		objA2btn2->Start();

		objA2->Finished.connect( this, &DialogWindow::WindowAnimDone );
	}
	break;
	case A3:
	{
		// create dialog A3
		if( !( layoutA3 = LoadLayout( dlgWdwArc, "my_DialogWindow_a3" ) ) )
		{
			return false;
		}

		ButtonCoords( Dlg_A3_Btn1, x, y, w, h );
		objA3 = new Object;
		objA3btn1 = new QuadButton( x, y, w, h );
		ButtonCoords( Dlg_A3_Btn2, x, y, w, h );
		objA3btn2 = new QuadButton( x, y, w, h );
		ButtonCoords( Dlg_A3_Btn3, x, y, w, h );
		objA3btn3 = new QuadButton( x, y, w, h );

		LOADANIM( "my_DialogWindow_a3_DialogIn", objA3, layoutA3 );				// window slides in from the bottom
		LOADANIM( "my_DialogWindow_a3_DialogOut", objA3, layoutA3 );			// window slides off the top of the page
		LOADANIM( "my_DialogWindow_a3_FocusBtn_off", objA3btn1, layoutA3 );		// button over/out/click
		LOADANIM( "my_DialogWindow_a3_FocusBtn_on", objA3btn1, layoutA3 );
		LOADANIM( "my_DialogWindow_a3_SelectBtn_Ac", objA3btn1, layoutA3 );

		objA3->BindGroup( layoutA3->FindGroup( "G_InOut" ) );
		objA3->SetAnimation( "my_DialogWindow_a3_DialogIn", 0, -1, -1, false );

		objA3btn1->BindGroup( layoutA3->FindGroup( "G_SelectBtnA" ) );
		objA3btn1->SetMouseOverAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_on" )->second, 0, -1 );
		objA3btn1->SetMouseOutAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_off" )->second, 0, -1 );
		objA3btn1->SetClickAnimation( brlans.find( "my_DialogWindow_a3_SelectBtn_Ac" )->second, 0, -1 );
		objA3btn1->SetTrigger( Button::Btn_A );

		objA3btn2->AddAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_on" )->second );
		objA3btn2->AddAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_off" )->second );
		objA3btn2->AddAnimation( brlans.find( "my_DialogWindow_a3_SelectBtn_Ac" )->second );
		objA3btn2->BindGroup( layoutA3->FindGroup( "G_SelectBtnB" ) );
		objA3btn2->SetMouseOverAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_on" )->second, 0, -1 );
		objA3btn2->SetMouseOutAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_off" )->second, 0, -1 );
		objA3btn2->SetClickAnimation( brlans.find( "my_DialogWindow_a3_SelectBtn_Ac" )->second, 0, -1 );
		objA3btn2->SetTrigger( Button::Btn_A );

		objA3btn3->AddAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_on" )->second );
		objA3btn3->AddAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_off" )->second );
		objA3btn3->AddAnimation( brlans.find( "my_DialogWindow_a3_SelectBtn_Ac" )->second );
		objA3btn3->BindGroup( layoutA3->FindGroup( "G_SelectBtnC" ) );
		objA3btn3->SetMouseOverAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_on" )->second, 0, -1 );
		objA3btn3->SetMouseOutAnimation( brlans.find( "my_DialogWindow_a3_FocusBtn_off" )->second, 0, -1 );
		objA3btn3->SetClickAnimation( brlans.find( "my_DialogWindow_a3_SelectBtn_Ac" )->second, 0, -1 );
		objA3btn3->SetTrigger( Button::Btn_A );

		objA3btn1->Clicked.connect( this, &DialogWindow::Button1Clicked );
		objA3btn2->Clicked.connect( this, &DialogWindow::Button2Clicked );
		objA3btn3->Clicked.connect( this, &DialogWindow::Button3Clicked );

		objA3->Start();
		objA3btn1->Start();
		objA3btn2->Start();
		objA3btn3->Start();

		objA3->Finished.connect( this, &DialogWindow::WindowAnimDone );
	}
	break;
	case B_1:
	case B_2:
	{
		// create dialog B
		if( !( layoutB = LoadLayout( dlgWdwArc, "my_DialogWindow_b" ) ) )
		{
			return false;
		}

		ButtonCoords( Dlg_B_Btn1, x, y, w, h );
		objB = new Object;
		objBbtn1 = new QuadButton( x, y, w, h );
		ButtonCoords( Dlg_B_Btn2, x, y, w, h );
		objBbtn2 = new QuadButton( x, y, w, h );

		LOADANIM( "my_DialogWindow_b_DialogIn", objB, layoutB );				// window slides in from the bottom
		LOADANIM( "my_DialogWindow_b_DialogOut", objB, layoutB );			// window slides off the top of the page
		LOADANIM( "my_DialogWindow_b_FocusBtn_off", objBbtn1, layoutB );		// button over/out/click
		LOADANIM( "my_DialogWindow_b_FocusBtn_on", objBbtn1, layoutB );
		LOADANIM( "my_DialogWindow_b_SelectBtn_Ac", objBbtn1, layoutB );

		objB->BindGroup( layoutB->FindGroup( "G_InOut" ) );
		objB->SetAnimation( "my_DialogWindow_b_DialogIn", 0, -1, -1, false );

		objBbtn1->BindGroup( layoutB->FindGroup( "G_SelectBtnA" ) );
		objBbtn1->SetMouseOverAnimation( brlans.find( "my_DialogWindow_b_FocusBtn_on" )->second, 0, -1 );
		objBbtn1->SetMouseOutAnimation( brlans.find( "my_DialogWindow_b_FocusBtn_off" )->second, 0, -1 );
		objBbtn1->SetClickAnimation( brlans.find( "my_DialogWindow_b_SelectBtn_Ac" )->second, 0, -1 );
		objBbtn1->SetAnimation( "my_DialogWindow_b_FocusBtn_off", 0, -1, -1, false );
		objBbtn1->SetTrigger( Button::Btn_A );

		objBbtn2->AddAnimation( brlans.find( "my_DialogWindow_b_FocusBtn_on" )->second );
		objBbtn2->AddAnimation( brlans.find( "my_DialogWindow_b_FocusBtn_off" )->second );
		objBbtn2->AddAnimation( brlans.find( "my_DialogWindow_b_SelectBtn_Ac" )->second );
		objBbtn2->BindGroup( layoutB->FindGroup( "G_SelectBtnB" ) );
		objBbtn2->SetMouseOverAnimation( brlans.find( "my_DialogWindow_b_FocusBtn_on" )->second, 0, -1 );
		objBbtn2->SetMouseOutAnimation( brlans.find( "my_DialogWindow_b_FocusBtn_off" )->second, 0, -1 );
		objBbtn2->SetClickAnimation( brlans.find( "my_DialogWindow_b_SelectBtn_Ac" )->second, 0, -1 );
		objBbtn2->SetAnimation( "my_DialogWindow_b_FocusBtn_off", 0, -1, -1, false );
		objBbtn2->SetTrigger( Button::Btn_A );

		objBbtn1->Clicked.connect( this, &DialogWindow::Button1Clicked );
		objBbtn2->Clicked.connect( this, &DialogWindow::Button2Clicked );

		if( type == B_2 )
		{
			SetPaneVisible( layoutB, "N_Top", false );
		}


		objB->Start();
		objBbtn1->Start();
		objBbtn2->Start();

		objB->Finished.connect( this, &DialogWindow::WindowAnimDone );
	}
	break;
	}

	loaded = true;

	this->type = type;
	choice = -1;
	return true;
}

void DialogWindow::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}

	switch( type )
	{
	case A0:
	{
		layoutA0->Render( modelview, ScreenProps, widescreen );
		objA0->Advance();
		objA0prog->Advance();
		objA0wait->Advance();
	}
	break;
	case A1:
	{
		layoutA1->Render( modelview, ScreenProps, widescreen );
		objA1->Advance();
		objA1btn1->Advance();
		objA1btn1->Update();
	}
	break;
	case A2:
	{
		layoutA2->Render( modelview, ScreenProps, widescreen );
		objA2->Advance();
		objA2btn1->Advance();
		objA2btn1->Update();
		objA2btn2->Advance();
		objA2btn2->Update();
	}
	break;
	case A3:
	{
		layoutA3->Render( modelview, ScreenProps, widescreen );
		objA3->Advance();
		objA3btn1->Advance();
		objA3btn1->Update();
		objA3btn2->Advance();
		objA3btn2->Update();
		objA3btn3->Advance();
		objA3btn3->Update();
	}
	break;
	case B_1:
	case B_2:
	{
		layoutB->Render( modelview, ScreenProps, widescreen );
		objB->Advance();
		objBbtn1->Advance();
		objBbtn1->Update();
		objBbtn2->Advance();
		objBbtn2->Update();
	}
	break;
	}
}

void DialogWindow::SetFadeIn()
{
	switch( type )
	{
	case A0:
	{
		objA0->SetAnimation( "my_DialogWindow_a0_DialogIn", 0, -1, -1, false );
		objA0->Start();
	}
	break;
	case A1:
	{
		objA1->SetAnimation( "my_DialogWindow_a1_DialogIn", 0, -1, -1, false );
		objA1->Start();
	}
	break;
	case A2:
	{
		objA2->SetAnimation( "my_DialogWindow_a2_DialogIn", 0, -1, -1, false );
		objA2->Start();
	}
	break;
	case A3:
	{
		objA3->SetAnimation( "my_DialogWindow_a3_DialogIn", 0, -1, -1, false );
		objA3->Start();
	}
	break;
	case B_1:
	case B_2:
	{
		objB->SetAnimation( "my_DialogWindow_b_DialogIn", 0, -1, -1, false );
		objB->Start();
	}
	break;
	}
}

void DialogWindow::SetFadeOut()
{
	switch( type )
	{
	case A0:
	{
		objA0->SetAnimation( "my_DialogWindow_a0_DialogOut", 0, -1, -1, false );
		objA0->Start();
	}
	break;
	case A1:
	{
		objA1->SetAnimation( "my_DialogWindow_a1_DialogOut", 0, -1, -1, false );
		objA1->Start();
	}
	break;
	case A2:
	{
		objA2->SetAnimation( "my_DialogWindow_a2_DialogOut", 0, -1, -1, false );
		objA2->Start();
	}
	break;
	case A3:
	{
		objA3->SetAnimation( "my_DialogWindow_a3_DialogOut", 0, -1, -1, false );
		objA3->Start();
	}
	break;
	case B_1:
	case B_2:
	{
		objB->SetAnimation( "my_DialogWindow_b_DialogOut", 0, -1, -1, false );
		objB->Start();
	}
	break;
	}
}

void DialogWindow::SetText( const char16 *message, const char16* btn1, const char16* btn2, const char16* btn3 )
{
	Layout *layout = NULL;

	switch( type )
	{
	case A0: layout = layoutA0; break;
	case A1: layout = layoutA1; break;
	case A2: layout = layoutA2; break;
	case A3: layout = layoutA3; break;
	case B_1:
	case B_2: layout = layoutB; break;
	}
	SystemMenuResource::SetText( layout, "T_Dialog", message );
	SystemMenuResource::SetText( layout, "T_BtnA", btn1 );
	SystemMenuResource::SetText( layout, "T_BtnB", btn2, true );
	SystemMenuResource::SetText( layout, "T_BtnC", btn3, true );
}

void DialogWindow::Button1Clicked()
{
	choice = 0;
	SetFadeOut();
}

void DialogWindow::Button2Clicked()
{
	choice = 1;
	SetFadeOut();
}

void DialogWindow::Button3Clicked()
{
	choice = 2;
	SetFadeOut();
}

void DialogWindow::WindowAnimDone()
{
	if( choice != -1 )
	{
		ButtonClicked( choice );
		choice = -1;
	}
}
