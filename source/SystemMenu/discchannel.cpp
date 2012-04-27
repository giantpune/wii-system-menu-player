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
#include "bmg.h"
#include "discchannel.h"
#include "gecko.h"
#include "Inputs.h"

DiscChannel::DiscChannel()
	: layout( NULL ),
	  testObj( new Object ),
	  wiiDisc( new Object ),
	  gcDisc( new Object )
{
}


DiscChannel::~DiscChannel()
{
	delete wiiDisc;
	delete gcDisc;
	delete testObj;
	delete layout;
	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}


bool DiscChannel::Load( const u8* diskBannAshData, u32 diskBannAshSize )
{
	if( loaded )
	{
		return true;
	}

	if( !diskBannAshData || !diskBannAshSize )
	{
		return false;
	}

	U8Archive arc( diskBannAshData, diskBannAshSize );
	if( !( layout = LoadLayout( arc, "my_DiskCh_a" ) ) )
	{
		return false;
	}

#define LOADANIM( x )										\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc, x ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ x ] = anim;									\
		testObj->AddAnimation( anim );						\
		wiiDisc->AddAnimation( anim );						\
		gcDisc->AddAnimation( anim );						\
	}														\
	while( 0 )

	// load animations
	Animation *anim;

	// read brlan files
	LOADANIM( "my_DiskCh_a_DiskEject" );
	LOADANIM( "my_DiskCh_a_DiskEnd" );
	LOADANIM( "my_DiskCh_a_DiskIn" );
	LOADANIM( "my_DiskCh_a_DiskLoop" );
	LOADANIM( "my_DiskCh_a_DiskLost" );
	LOADANIM( "my_DiskCh_a_DiskStart" );
	LOADANIM( "my_DiskCh_a_Start" );
	LOADANIM( "my_DiskCh_a_Unknown" );
	LOADANIM(  "my_DiskCh_a_UnknownLoop" );
	LOADANIM( "my_DiskCh_a_UnknwnEject" );

	// bind materials
	foreach( Material *mat, layout->Materials() )
	{
		const char* name = mat->getName();
		if( strstr( name, "Wii" ) )
		{
			wiiDisc->BindMaterial( mat );
		}
		else if( strstr( name, "GC" ) )
		{
			gcDisc->BindMaterial( mat );
		}
		else
		{
			testObj->BindMaterial( mat );
		}
	}

	// bind panes
	//testObj->BindPane( layout->FindPane( "RootPane" ) );
	testObj->BindPane( layout->FindPane( "N_Back" ) );
	testObj->BindPane( layout->FindPane( "N_Bar" ) );
	testObj->BindPane( layout->FindPane( "BackMask2" ) );
	//testObj->BindPane( layout->FindPane( "N_Shade0" ) );
	testObj->BindPane( layout->FindPane( "N_Ref0" ) );
	testObj->BindPane( layout->FindPane( "T_Comment0" ) );
	testObj->BindPane( layout->FindPane( "T_Comment1" ) );
	testObj->BindPane( layout->FindPane( "N_Unknown" ) );
	testObj->BindPane( layout->FindPane( "N_Ref0_00" ) );
	//testObj->UnbindPane( layout->FindPane( "N_Disk" ) );

	Pane *p;
#define MOVEPANE( x, y )									\
	do														\
	{														\
		if( (p = layout->FindPane( y ) ) )					\
		{													\
			testObj->UnbindPane( p );						\
			x->BindPane( p );								\
		}													\
	}														\
	while( 0 )

	MOVEPANE( wiiDisc, "N_Wii0" );
	MOVEPANE( wiiDisc, "ShadeWii" );
	MOVEPANE( wiiDisc, "N_RefWii" );
	MOVEPANE( wiiDisc, "W_Wii" );

	MOVEPANE( gcDisc, "N_GC0" );
	MOVEPANE( gcDisc, "ShadeGC" );
	MOVEPANE( gcDisc, "N_RefGC" );
	MOVEPANE( gcDisc, "W_GC" );
	MOVEPANE( gcDisc, "GCDisk" );

	testObj->SetAnimation( "my_DiskCh_a_DiskEject" );
	testObj->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	testObj->Start();

	wiiDisc->SetAnimation( "my_DiskCh_a_DiskEject" );
	wiiDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	wiiDisc->Start();

	gcDisc->SetAnimation( "my_DiskCh_a_DiskEject" );
	gcDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	gcDisc->Start();

	//gcDisc->Debug();
	//wiiDisc->Debug();
	//testObj->Debug();

	gcDisc->Finished.connect( this, &DiscChannel::GCDiscAnimDone );
	wiiDisc->Finished.connect( this, &DiscChannel::WiiDiscAnimDone );
	gcDisc->AnimationStarted.connect( this, &DiscChannel::GCDiscAnimStarted );
	wiiDisc->AnimationStarted.connect( this, &DiscChannel::WiiDiscAnimStarted );
	testObj->Finished.connect( this, &DiscChannel::BackgroundAnimDone );

	SetText( layout, "T_Bar", 0 );// "Disc Channel"
	SetText( layout, "T_Comment0", 4 );// "Please insert a disc."
	SetText( layout, "T_Comment1", 6 );// unable to read the disc

	loaded = true;
	state = St_Idle_Nodisc;
	return true;
}

void DiscChannel::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}

	layout->Render( modelview, ScreenProps, widescreen );
	testObj->Advance();
	gcDisc->Advance();
	wiiDisc->Advance();
}

void DiscChannel::JumpToUnknownDiscAnim()
{
	testObj->ClearAnimStack();
	wiiDisc->ClearAnimStack();
	gcDisc->ClearAnimStack();

	testObj->SetAnimation( "my_DiskCh_a_DiskEject" );
	testObj->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	testObj->Start();

	wiiDisc->SetAnimation( "my_DiskCh_a_DiskEject" );
	wiiDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	wiiDisc->Start();

	gcDisc->SetAnimation( "my_DiskCh_a_DiskEject" );
	gcDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	gcDisc->Start();
	state = St_Unknown;
}

void DiscChannel::Reset()
{
	state = St_Idle_Nodisc;
	testObj->ClearAnimStack();
	testObj->SetAnimation( "my_DiskCh_a_DiskEject" );
	testObj->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	testObj->Start();

	wiiDisc->ClearAnimStack();
	wiiDisc->SetAnimation( "my_DiskCh_a_DiskEject" );
	wiiDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	wiiDisc->Start();

	gcDisc->ClearAnimStack();
	gcDisc->SetAnimation( "my_DiskCh_a_DiskEject" );
	gcDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
	gcDisc->Start();

	state = St_Idle_Nodisc;
}

void DiscChannel::StartReadingDisc()
{
	if( state == St_Idle_Nodisc )
	{
		// animate everything that isnt a disc to make the text fade out and stuff like that
		testObj->ScheduleAnimation( "my_DiskCh_a_DiskStart", 0, 50, 50, false );// fadeout text
		testObj->Start();

		// start both discs spinning
		wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskStart", 0, 50 );// spinup
		wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskLoop", 0, 15 );// spinning loop
		wiiDisc->Start();

		gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskStart", 0, 50 );// spinup
		gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskLoop", 0, 15 );// spinning loop
		gcDisc->Start();

		// set state
		state = St_DiscSpinLoop;
	}
}

void DiscChannel::DiscIsWii()
{
	if( state != St_DiscSpinLoop )
	{
		//gprintf( "DiscChannel::DiscIsWii(): called without disc spinup : %u %u\n", St_DiscSpinLoop, state );
		return;
	}
	gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskLost", 0, 170, -1, false );// fade out GC disc
	state = St_GCFadeOut;
}

void DiscChannel::DiscIsGC()
{
	if( state != St_DiscSpinLoop )
	{
		//gprintf( "DiscChannel::DiscIsGC(): called without disc spinup\n" );
		return;
	}
	wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskLost", 0, 170, -1, false );// fade out Wii disc
	state = St_WiiFadeOut;
}

void DiscChannel::GCDiscAnimDone()
{
	if( state == St_GCFadeOut )
	{
		wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskIn", 0, 140, 140, false );// insert wii disc
		state = St_InsertWiiDisc;
	}
	else if( state == St_InsertGCDisc )
	{
		Finished();
	}
}

void DiscChannel::WiiDiscAnimDone()
{
	if( state == St_WiiFadeOut )
	{
		gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskIn", 0, 140, 140, false );// insert gc disc
		state = St_InsertGCDisc;
	}
	else if( state == St_InsertWiiDisc )
	{
		Finished();
	}
	else if( state == St_FadeoutWiiAndGC )
	{
		testObj->ScheduleAnimation( "my_DiskCh_a_Unknown" );// start showing unknown disc
		testObj->ScheduleAnimation( "my_DiskCh_a_UnknownLoop", 0, 150, -1, true, true );// unknown disc looping anim
		testObj->Start();
		state = St_Unknown;
	}
}

void DiscChannel::WiiDiscAnimStarted( AnimationLink * )
{
	if( state == St_InsertWiiDisc )// synchronize fading background with spinning disc
	{
		testObj->SetAnimation( "my_DiskCh_a_DiskIn", 0, 140, -1, false );
		testObj->Start();
	}
}

void DiscChannel::GCDiscAnimStarted( AnimationLink * )
{
	if( state == St_InsertGCDisc )// synchronize fading background with spinning disc
	{
		testObj->SetAnimation( "my_DiskCh_a_DiskIn", 0, 140, -1, false );
		testObj->Start();
	}
}

void DiscChannel::DiscIsUnknown()
{
	if( state != St_DiscSpinLoop )
	{
		gprintf( "DiscChannel::DiscIsUnknown(): called without disc spinup" );
		return;
	}
	wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskLost", 0, 170, -1, false );// fade out Wii disc
	gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskLost", 0, 170, -1, false );// fade out Wii disc
	state = St_FadeoutWiiAndGC;
}

void DiscChannel::BackgroundAnimDone()
{
	if( state == St_FadeOutUnknown )
	{
		wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskEject" );
		wiiDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
		wiiDisc->Start();

		gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskEject" );
		gcDisc->ScheduleAnimation( "my_DiskCh_a_Start", 0, 140, 140, false );
		gcDisc->Start();
		state = St_Idle_Nodisc;
	}
}

void DiscChannel::EjectDisc()
{
	if( state == St_DiscSpinLoop )
	{
		// stop spinning wii and GC disc
		gcDisc->ScheduleAnimation( "my_DiskCh_a_DiskEnd", 0, -1, -1, false );
		wiiDisc->ScheduleAnimation( "my_DiskCh_a_DiskEnd", 0, -1, -1, false );

		testObj->ScheduleAnimation( "my_DiskCh_a_UnknwnEject", 45, 200, -1, false ); // fade in text
		testObj->Start();
		state = St_Idle_Nodisc;
	}
	else if( state == St_Unknown )
	{
		testObj->ScheduleAnimation( "my_DiskCh_a_UnknwnEject", 0, 200, -1, false ); // eject unknown disc
		testObj->Start();
		state = St_FadeOutUnknown;
	}
}
