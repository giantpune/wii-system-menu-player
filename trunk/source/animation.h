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
#ifndef ANIMATION_H
#define ANIMATION_H

#include "Layout.h"
#include "Material.h"
#include "Pane.h"

// this is a class to access data from within a brlan
class Animation
{
public:
	static const u32 MAGIC_ANIMATION = MAKE_FOURCC('R', 'L', 'A', 'N');
	static const u32 MAGIC_PANE_ANIMATION_INFO = MAKE_FOURCC('p', 'a', 'i', '1');

	Animation( const std::string &name )
		: name( name ),
		  header( NULL ),
		  paiHeader( NULL )
	{
	}

	void Load( const RLAN_Header *header );
	const std::string &GetName() const { return name; }

	// texture file names used for palette animations
	std::vector< std::string >paletteNames;

	// number of frames
	float FrameCount() const { return paiHeader ? paiHeader->frame_count : 0.0f; }

	// load keyframes for pane/material
	//! clears existing key frames
	bool LoadKeyFrames( Pane *a );
	bool LoadKeyFrames( Material *a );

	// check if the brlan contains animations for a given thingy
	bool HaveAnimationsFor( Pane *a );
	bool HaveAnimationsFor( Material *a );

	bool Loop(){ return paiHeader ? paiHeader->loop : false; }

protected:
	// name of brlan file used to create this class instance, ie "banner_loop.brlan"
	std::string name;

	// pointer to brlan header
	RLAN_Header *header;

	// animation section header
	PAI1_Header *paiHeader;
};

// this is a class used to link animations together with panes and materials
// it is responsible for keeping track of the current frame number and stuff like that
class AnimationLink
{
public:
	enum Loop
	{
		LoopNo = 0,
		LoopYes = 1,
		LoopRespectBrlanHeader = 2// something about this one doesn't act quite right
	};

	AnimationLink( Animation* animation, FrameNumber startFrame = 0.f, FrameNumber endFrame = -1.f,
				   FrameNumber waitFrame = -1.f, Loop loop = LoopRespectBrlanHeader, bool forwardAndBackward = false );

	~AnimationLink();

	bool BindPane( Pane *pane );
	void UnbindPane( Pane *pane );
	bool BindMaterial( Material *mat );
	void UnbindMaterial( Material *mat );

	void SetFrame( FrameNumber num );
	void AdvanceFrame();
	bool IsFinished() const { return animDone; }

	bool IsLooping() const { return loop; }
	void SetLoop( bool loop ){ this->loop = loop; }

	Animation *GetAnimation(){ return anim; }

	// meh, just make these public
	Animation *anim;
	FrameNumber startFrame;
	FrameNumber endFrame;
	FrameNumber waitFrame;
	FrameNumber currentFrame;
	bool loop;
	bool forwardAndBackward;// some animations seem to play forward and reverse
	int direction;
	bool animDone;

protected:

	// lists of panes and materials this class will animate
	std::vector< Material * >bindedMaterials;
	std::vector< Pane * >bindedPanes;

	std::vector< Pane * >::iterator FindPane( Pane *pane );
	std::vector< Material * >::iterator FindMaterial( Material *mat );

};

#endif // ANIMATION_H
