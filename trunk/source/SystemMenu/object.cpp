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
#include "gecko.h"
#include "object.h"

Object::Object()
	: animationLink( NULL ),
	  animating( false )
{
}

Object::~Object()
{
	foreach( AnimStackEntry *entry, animStack )
	{
		delete entry;
	}
	delete animationLink;
}

void Object::ClearAnimStack()
{
	foreach( AnimStackEntry *entry, animStack )
	{
		delete entry;
	}
	animStack.clear();
}

void Object::BindPanes( const PaneList &paneList, bool recurse )
{
	foreach( Pane *pane, paneList )
	{
		BindPane( pane, recurse );
	}
}

void Object::BindPane( Pane *pane, bool recurse )
{
	if( !pane || panes.Contains( pane ) )
	{
		return;
	}
	//gprintf( "Object::BindPane( \"%s\" )\n", pane->getName() );

	if( animationLink )
	{
		animationLink->BindPane( pane );
	}
	panes.push_back( pane );
	if( recurse )
	{
		foreach( Pane *pane2, pane->panes )
		{
			BindPane( pane2, true );
		}
	}
}

void Object::BindMaterials( const MaterialList &matList )
{
	foreach( Material *mat, matList )
	{
		BindMaterial( mat );
	}
}

void Object::BindMaterial( Material *mat )
{
	if( !mat || materials.Contains( mat ) )
	{
		return;
	}
	materials << mat;
	if( animationLink )
	{
		animationLink->BindMaterial( mat );
	}
}

void Object::UnbindMaterial( Material *mat )
{
	if( !mat )
	{
		return;
	}
	materials.Remove( mat );
	if( animationLink )
	{
		animationLink->UnbindMaterial( mat );
	}
}

void Object::Debug()
{
	gprintf( "Object::Debug:\n" );
	gprintf( "  anim: %s\n", ( animationLink && animationLink->anim ) ? animationLink->anim->GetName().c_str() : "<NULL>" );
	gprintf( "  materials %u\n", materials.size() );
	foreach( Material *mat, materials )
	{

		gprintf( "    %s\n", mat->getName() );
	}
	gprintf( "  panes %u\n", panes.size() );
	foreach( Pane *pane, panes )
	{

		gprintf( "    %s\n", pane->getName() );
	}
}

void Object::BindGroup( const Layout::Group *group )
{
	if( !group )
	{
		return;
	}
	BindPanes( group->panes, true );
}

void Object::UnbindPanes( const PaneList &paneList, bool recurse )
{
	foreach( Pane *pane, paneList )
	{
		UnbindPane( pane, recurse );
	}
}

void Object::UnbindAllPanes()
{
	if( animationLink )
	{
		foreach( Pane *pane, panes )
		{
			animationLink->UnbindPane( pane );
		}
	}
	panes.clear();

}

void Object::UnbindPane( Pane *pane, bool recurse )
{
	for( u32 i = 0; i < panes.size(); i++ )
	{
		if( panes[ i ] == pane )
		{
			panes.erase( panes.begin() + i );
			break;
		}
	}
	if( animationLink )
	{
		animationLink->UnbindPane( pane );
	}
	if( recurse )
	{
		foreach( Pane *pane2, pane->panes )
		{
			UnbindPane( pane2, true );
		}
	}
}

void Object::UnbindGroup( Layout::Group *group )
{
	if( !group )
	{
		return;
	}
	UnbindPanes( group->panes, true );
}

Animation* Object::FindBrlan( const std::string &str )
{
	//gprintf( "FindBrlan( \"%s\" ): %u\n", str.c_str(), brlans.size() );
	foreach( Animation *brlan, brlans )
	{
		if( brlan->GetName() == str )
		{
			return brlan;
		}
	}
	return NULL;
}

void Object::AddAnimation( Animation *anim )
{
	foreach( Animation *brlan, brlans )
	{
		if( anim == brlan )
		{
			return;
		}
	}
	brlans << anim;
}

void Object::SetAnimation( const std::string &brlanName, FrameNumber start, FrameNumber end, FrameNumber wait,
				   bool loop, bool forwardAndReverse )
{
	ClearAnimStack();
	Animation* brlan = FindBrlan( brlanName );
	if( !brlan )
	{
		gprintf( "brlan not found: %s\n", brlanName.c_str() );
		return;
	}
	delete animationLink;
	animationLink = new AnimationLink( brlan, start, end, wait,
									   loop ? AnimationLink::LoopYes : AnimationLink::LoopNo, forwardAndReverse );
	foreach( Pane *pane, panes )
	{
		//gprintf( "trying to bind pane: %s\n", pane->getName() );
		animationLink->BindPane( pane );
	}
	foreach( Material *mat, materials )
	{
		animationLink->BindMaterial( mat );
	}
	AnimationStarted( animationLink );
}

void Object::ScheduleAnimation( const std::string &brlanName, FrameNumber start, FrameNumber end, FrameNumber wait,
								bool loop, bool forwardAndReverse )
{
	//gprintf( "ScheduleAnimation( \"%s\", %.0f, %.0f, %.0f, %u %u ): %u\n"
	//		 , brlanName.c_str(), start, end, wait, loop, forwardAndReverse, animationLink != 0 );
	Animation* brlan = FindBrlan( brlanName );
	if( !brlan )
	{
		gprintf( "brlan not found: %s\n", brlanName.c_str() );
		return;
	}

	// not currently animating, set this one as current
	if( !animationLink )
	{
		animationLink = new AnimationLink( brlan, start, end, wait,
										   loop ? AnimationLink::LoopYes : AnimationLink::LoopNo, forwardAndReverse );
		foreach( Pane *pane, panes )
		{
			animationLink->BindPane( pane );
		}
		foreach( Material *mat, materials )
		{
			animationLink->BindMaterial( mat );
		}
		AnimationStarted( animationLink );
		return;
	}

	// create a new entry and add it to the list
	AnimStackEntry *entry = new AnimStackEntry( brlan, start, end, wait,
												loop ? AnimationLink::LoopYes : AnimationLink::LoopNo, forwardAndReverse );
	if( animStack.size() )
	{
		animStack[ animStack.size() - 1 ]->loop = AnimationLink::LoopNo;
	}
	animStack << entry;
	animationLink->loop = AnimationLink::LoopNo;
}

void Object::Restart()
{
	if( animationLink )
	{
		animationLink->SetFrame( animationLink->startFrame );
	}
}

void Object::Advance()
{
	if( !animating || !animationLink )
	{
		//gprintf( "animating: %u animationLink: %p\n", animating, animationLink );
		return;
	}
	animationLink->AdvanceFrame();
	/*gprintf( "animationLink->AdvanceFrame(): %.2f %.2f %.2f %u %s\n",
			 animationLink->currentFrame,
			 animationLink->startFrame,
			 animationLink->endFrame,
			 ( animationLink->loop == AnimationLink::LoopYes ),
			 animationLink->anim->GetName().c_str() );*/
	if( !animationLink->IsFinished() )
	{
		return;
	}
	if( !animStack.size() )
	{
		animating = false;
		// send finished signal
		Finished();
		return;
	}
	AnimStackEntry *e = animStack.TakeFirst();
	delete animationLink;
	animationLink = new AnimationLink( e->brlan, e->startFrame, e->endFrame, e->waitFrame, e->loop, e->forwardAndBackward );

	delete e;
	foreach( Pane *pane, panes )
	{
		//gprintf( "trying to bind pane: %s\n", pane->getName() );
		animationLink->BindPane( pane );
	}
	foreach( Material *mat, materials )
	{
		animationLink->BindMaterial( mat );
	}
	AnimationStarted( animationLink );
}
