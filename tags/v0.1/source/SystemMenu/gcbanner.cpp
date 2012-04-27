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
#include "gcbanner.h"

GCBanner::GCBanner()
	: lyt( NULL ),
	  obj( new Object ),
	  lan( NULL )
{
}

GCBanner::~GCBanner()
{
	delete obj;
	delete lan;
	delete lyt;
}

bool GCBanner::Load( const u8* GCBannAshData, u32 GCBannAshSize )
{
	if( loaded )
	{
		return true;
	}

	if( !GCBannAshData || !GCBannAshSize )
	{
		return false;
	}

	U8Archive arc( GCBannAshData, GCBannAshSize );
	if( !( lyt = LoadLayout( arc, "my_GCTop_a" ) ) )
	{
		return false;
	}

	if( !( lan = LoadAnimation( arc, "my_GCTop_a_BackLoop" ) ) )
	{
		return false;
	}

	obj->BindPane( lyt->FindPane( "RootPane" ) );
	obj->BindMaterials( lyt->Materials() );
	obj->AddAnimation( lan );
	obj->SetAnimation( "my_GCTop_a_BackLoop" );
	obj->Start();

	loaded = true;
	return true;
}

void GCBanner::Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen )
{
	if( !loaded )
	{
		return;
	}
	lyt->Render( modelview, ScreenProps, widescreen );
	obj->Advance();
}

