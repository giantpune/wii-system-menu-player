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
#ifndef OBJECT_H
#define OBJECT_H

#include "animation.h"
#include "Layout.h"
#include "utils/list.h"
#include "utils/sigslot.h"

// handles a set of panes, linking it to different animations,
// and updating the animation.  right now, it has nothing to do
// with drawing, only determining what frames to set

// the idea behind this is because there are different sections
// of layouts that need to have different animations at different times
class Object : public sigslot::has_slots<>
{
public:
	Object();
	~Object();

	// add or remove panes to this list of pnaes to animate
	void BindPanes( const PaneList &paneList, bool recurse = true );
	void BindPane( Pane *pane, bool recurse = true );
	void BindGroup( const Layout::Group *group );
	void UnbindPanes( const PaneList &paneList, bool recurse = true );
	void UnbindPane( Pane *pane, bool recurse = true );
	void UnbindGroup( Layout::Group *group );
	void UnbindAllPanes();
	void BindMaterial( Material *mat );
	void UnbindMaterial( Material *mat );
	void BindMaterials( const MaterialList &matList );

	// set the resources used for the animation links
	//void SetResourceBlock( Resources *resources );

	// add an animation to the list
	void AddAnimation( Animation *anim );


	// sets the current animation
	void SetAnimation( const std::string &brlan, FrameNumber start = 0.f,
					   FrameNumber end = -1.f, FrameNumber wait = -1.f,
					   bool loop = true, bool forwardAndReverse = false );

	// schedules an animation to happen after the current one has finished
	void ScheduleAnimation( const std::string &brlan, FrameNumber start = 0.f,
					   FrameNumber end = -1.f, FrameNumber wait = -1.f,
					   bool loop = true, bool forwardAndReverse = false );

	// applies the next frame in the current animation to each pane
	void Advance();

	void Pause( bool paused ){ animating = !paused; }
	void Start(){ animating = true;}
	void Restart();
	void SetLoop( bool loop ){ if( animationLink ) animationLink->loop = loop; }
	void SetFrame( FrameNumber frame ) { if( animationLink ) animationLink->SetFrame( frame ); }

	FrameNumber GetFrame() const { return animationLink ? animationLink->currentFrame: 0; }

	// are all the current animations finished?
	bool IsFinished() const{ return animationLink ? animationLink->animDone : true; }


	void ClearAnimStack();

	// debugging stuff
	void Debug();


	// signals
	sigslot::signal1< AnimationLink * > AnimationStarted;
	sigslot::signal0<> Finished;

protected:
	// loaded brlan files
	List< Animation* >brlans;
	Animation* FindBrlan( const std::string &str );

	// resources used for drawing panes
	//Resources *resourceBlock;

	// animation links between brlans and panes
	AnimationLink *animationLink;

	// list of panes to draw
	List< Pane * >panes;

	// material list to animate
	List< Material*> materials;

	bool animating;

	// something to keep track of which animations to play next
	struct AnimStackEntry
	{
		// brlan file to use
		Animation* brlan;

		FrameNumber startFrame;
		FrameNumber endFrame;
		FrameNumber waitFrame;
		AnimationLink::Loop loop;
		bool forwardAndBackward;



		AnimStackEntry( Animation *brlan,
						FrameNumber startFrame = 0.f,
						FrameNumber endFrame = -1.f, FrameNumber waitFrame = -1.f,
						AnimationLink::Loop loop = AnimationLink::LoopYes,
						bool forwardAndBackward = false )
			: brlan( brlan ),
			  startFrame( startFrame ),
			  endFrame( endFrame ),
			  waitFrame( waitFrame ),
			  loop( loop ),
			  forwardAndBackward( forwardAndBackward )
		{
		}
	};
	List< AnimStackEntry * >animStack;

};

#endif // OBJECT_H
