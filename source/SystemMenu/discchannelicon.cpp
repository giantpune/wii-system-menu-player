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
#include "discchannelicon.h"
#include "sc.h"

DiscChannelIcon::DiscChannelIcon()
	: lyt1( NULL ),
	  lyt2( NULL ),
	  lyt3( NULL ),

	  obj1( new Object ),
	  obj2( new Object ),
	  obj3( new Object )
{
}

DiscChannelIcon::~DiscChannelIcon()
{
	delete obj1;
	delete obj2;
	delete obj3;

	delete lyt1;
	delete lyt2;
	delete lyt3;

	std::map< std::string, Animation *>:: iterator it = brlans.begin(), itE = brlans.end();
	while( it != itE )
	{
		delete it->second;
		++it;
	}
}

bool DiscChannelIcon::Load( const u8* diskThumbAshData, u32 diskThumbAshSize )
{
	if( loaded )
	{
		return true;
	}

	if( !diskThumbAshData || !diskThumbAshSize )
	{
		return false;
	}

	U8Archive arc( diskThumbAshData, diskThumbAshSize );
	if( !( lyt1 = LoadLayout( arc, "my_DiskCh_b" ) ) )
	{
		return false;
	}
	if( !( lyt2 = LoadLayout( arc, "my_DiskCh_in" ) ) )
	{
		return false;
	}
	if( !( lyt3 = LoadLayout( arc, "my_GCIcon_a" ) ) )
	{
		return false;
	}
	lyt3->SetLanguage( CONF_GetLanguageString() );

	// load animations
	Animation *anim;
#define LOADANIM( x, y )									\
	do														\
	{														\
		if( !(anim = LoadAnimation( arc, y ) ) )			\
		{													\
			return false;									\
		}													\
		brlans[ y ] = anim;									\
		x->AddAnimation( anim );							\
	}														\
	while( 0 )

	LOADANIM( obj1, "my_DiskCh_b" );						// this is the main looping icon with a white disc spinning around
	LOADANIM( obj2, "my_DiskCh_In_DiskIn" );				// this is the grey disc that flies in from the left of the screen
	LOADANIM( obj2, "my_DiskCh_In_DiskOut" );
	LOADANIM( obj3, "my_GCIcon_a" );						// this is the "theres an update" and "Gamecube" layout and animation

	obj1->BindPane( lyt1->FindPane( "RootPane" ) );
	obj2->BindPane( lyt2->FindPane( "RootPane" ) );
	obj2->BindMaterials( lyt2->Materials() );
	obj3->BindPane( lyt3->FindPane( "RootPane" ) );
	obj3->BindMaterials( lyt3->Materials() );

	obj1->SetAnimation( "my_DiskCh_b" );
	obj1->Start();


	//obj2->SetAnimation( "my_DiskCh_In_DiskOut" );
	//obj2->Start();

	obj3->SetAnimation( "my_GCIcon_a" );
	obj3->Start();

	SetPaneVisible(lyt3, "N_DiscUpdateIcon", false );

	// use widescreen mask
	if( _CONF_GetAspectRatio() == 1 )
	{
		Material *mat;
		if( ( mat = lyt2->FindMaterial( "DiskIn" ) ) )
		{
			mat->SetTextureIndex( 0, 0 );
		}
	}




	loaded = true;
	return true;
}

void DiscChannelIcon::StartInsertDiscAnim()
{
	obj2->SetAnimation( "my_DiskCh_In_DiskIn", 0, -1, -1, false );
	obj2->Start();
}

void DiscChannelIcon::StartEjectDiscAnim()
{
	obj2->SetAnimation( "my_DiskCh_In_DiskOut", 0, -1, -1, false );
	obj2->Start();
}

/*void DiscChannelIcon::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}
	//lyt1->Render( modelview, ScreenProps, widescreen );
	//obj1->Advance();

	//lyt2->Render( modelview, ScreenProps, widescreen );
	//obj2->Advance();

	//lyt3->Render( modelview, ScreenProps, widescreen );
	//obj3->Advance();
}*/

