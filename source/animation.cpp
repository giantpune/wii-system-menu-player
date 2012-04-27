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
#include "animation.h"

// load keyframes from a brlan file

void Animation::Load( const RLAN_Header *header )
{
	if( !header || header->magic != MAGIC_ANIMATION || header->endian != 0xFEFF || header->version != 0x0008 )
	{
		gprintf( "Animation::Load(): bad shit  %p\n", header );
		return;
	}
	this->header = const_cast< RLAN_Header * >( header );

	const u8 *position = ((const u8 *) header) + header->offset;

	// this seems to always be the case
	if( header->section_count != 1 )
	{
		gprintf( "Animation::Load(): header->section_count != 1\n" );
		return;
	}
	paiHeader = (PAI1_Header *)position;
	if( paiHeader->magic != MAGIC_PANE_ANIMATION_INFO )
	{
		paiHeader = NULL;
		gprintf( "Animation::Load(): paiHeader->magic != MAGIC_PANE_ANIMATION_INFO\n" );
		return;
	}
	// get filenames for tpl palette animations
	const u32 *nameOffsets = (const u32 *)(paiHeader + 1);
	for(u32 i = 0; i < paiHeader->file_count; i++)
	{
		const char* name = (((const char *) nameOffsets) + nameOffsets[i]);
		//gprintf( "adding palette name: %s\n", name );
		paletteNames.push_back( name );
	}
}

bool Animation::HaveAnimationsFor( Pane *a )
{
	if( !a || !paiHeader )
	{
		gprintf( "Animation::HaveAnimationsFor(): !a || !paiHeader %p %p\n", a, paiHeader );
		return false;
	}

	const u32 *offsets = (const u32 *) (((const u8 *)paiHeader) + paiHeader->entry_offset);

	const char *pName = a->getName();
	for(u32 n = 0; n < paiHeader->animator_count; n++)
	{
		const AnimatorHeader *animHdr = (const AnimatorHeader *) (((const u8 *)paiHeader ) + offsets[n]);
		if( animHdr->is_material || strncmp( pName, animHdr->name, 20 ) )
		{
			continue;
		}
		return true;
	}
	return false;
}

bool Animation::HaveAnimationsFor( Material *a )
{
	if( !a || !paiHeader )
	{
		return false;
	}

	const u32 *offsets = (const u32 *) (((const u8 *)paiHeader) + paiHeader->entry_offset);

	const char *pName = a->getName();
	for(u32 n = 0; n < paiHeader->animator_count; n++)
	{
		const AnimatorHeader *animHdr = (const AnimatorHeader *) (((const u8 *)paiHeader ) + offsets[n]);
		if( !animHdr->is_material || strncmp( pName, animHdr->name, 20 ) )
		{
			continue;
		}
		return true;
	}
	return false;
}

bool Animation::LoadKeyFrames( Pane *a )
{
	if( !a || !paiHeader )
	{
		return false;
	}

	//gprintf( "clear pane keyframes: %s\n", a->getName() );
	//a->ClearKeyFrames();

	bool ret = false;
	const u32 *offsets = (const u32 *) (((const u8 *)paiHeader) + paiHeader->entry_offset);

	const char *pName = a->getName();
	for(u32 n = 0; n < paiHeader->animator_count; n++)
	{
		const AnimatorHeader *animHdr = (const AnimatorHeader *) (((const u8 *)paiHeader ) + offsets[n]);
		if( animHdr->is_material || strncmp( pName, animHdr->name, 20 ) )
		{
			continue;
		}
		if( !ret )
		{
			//gprintf( "clear pane keyframes: %s\n", a->getName() );
			a->ClearKeyFrames();
			ret = true;
		}
		a->LoadKeyFrames( (const u8 *) animHdr, animHdr->tag_count, sizeof(AnimatorHeader) );
	}
	return ret;
}

bool Animation::LoadKeyFrames( Material *a )
{
	if( !a || !paiHeader )
	{
		return false;
	}

	bool ret = false;
	const u32 *offsets = (const u32 *) (((const u8 *)paiHeader) + paiHeader->entry_offset);

	const char *pName = a->getName();
	for(u32 n = 0; n < paiHeader->animator_count; n++)
	{
		const AnimatorHeader *animHdr = (const AnimatorHeader *) (((const u8 *)paiHeader ) + offsets[n]);
		if( !animHdr->is_material || strncmp( pName, animHdr->name, 20 ) )
		{
			continue;
		}
		if( !ret )
		{
			//gprintf( "clear material keyframes: %s\n", a->getName() );
			a->ClearKeyFrames();
			ret = true;
		}
		//gprintf( "loading keyframes for \"%s\"\n", pName );
		a->LoadKeyFrames( (const u8 *) animHdr, animHdr->tag_count, sizeof(AnimatorHeader) );
	}
	return ret;
}

AnimationLink::AnimationLink( Animation* animation, FrameNumber startFr, FrameNumber endFr,
			   FrameNumber waitFr, Loop loop, bool forwardAndBackward )
	: anim( animation ),
	  startFrame( startFr ),
	  endFrame( endFr ),
	  waitFrame( waitFr ),
	  currentFrame( startFr ),
	  loop( false ),
	  forwardAndBackward( forwardAndBackward ),
	  direction( 1 ),
	  animDone( false )
{
	if( !anim )
	{
		gprintf( "AnimationLink(): anim is NULL.  don\'t do that." );
		return;
	}
	//gprintf( "AnimationLink( %.2f, %.2f, %.2f )\n", startFrame, endFrame, waitFrame );

	// handle -1 arguements
	if( endFrame < 0.f )
	{
		endFrame = anim->FrameCount() - 1;
	}
	if( waitFrame < 0.f )
	{
		waitFrame = endFrame;
	}

	// set loop
	switch( loop )
	{
	case LoopYes:
		this->loop = true;
		break;
	case LoopNo:
		this->loop = false;
		break;
	case LoopRespectBrlanHeader:
		this->loop = anim->Loop();
		break;
	}
	//gprintf( "forward and reverse: %u\n", forwardAndBackward );

	//gprintf( "AnimationLink2( %.2f, %.2f, %.2f )\n", startFrame, endFrame, waitFrame );
	//gprintf( "AnimationLink( %p, %.2f, %.2f, %.2f, %u, %u )\n"
	//		 , animation, startFrame, endFrame, waitFrame, loop, forwardAndBackward );
}

AnimationLink::~AnimationLink()
{
	while( bindedPanes.size() )
	{
		UnbindPane( *bindedPanes.begin() );
	}

	u32 cnt = bindedMaterials.size();
	for( u32 i = 0; i < cnt; i++ )
	{
		bindedMaterials[ i ]->animationLink = NULL;
	}
}

bool AnimationLink::BindPane( Pane *pane )
{
	if( anim
		&& FindPane( pane ) == bindedPanes.end()
		&& anim->HaveAnimationsFor( pane ) )
	{
		bindedPanes.push_back( pane );
		AnimationLink *currentLink = pane->animationLink;
		if( currentLink )
		{
			currentLink->UnbindPane( pane );
		}
		pane->animationLink = this;
		anim->LoadKeyFrames( pane );
		pane->SetFrame( currentFrame );
		//gprintf( "binded pane: %s to %s\n", pane->getName(), anim->GetName().c_str() );
		return true;
	}
	return false;
}

void AnimationLink::UnbindPane( Pane *pane )
{
	std::vector< Pane * >::iterator it = FindPane( pane );
	if( it != bindedPanes.end() )
	{
		bindedPanes.erase( it );
	}
	if( pane->animationLink == this )
	{
		pane->animationLink = NULL;
		pane->ClearKeyFrames();
	}
	for( u32 i = 0; i < pane->panes.size(); i++ )
	{
		UnbindPane( pane->panes[ i ] );
	}
}

bool AnimationLink::BindMaterial( Material *mat )
{
	if( anim
		&& FindMaterial( mat ) == bindedMaterials.end()
		&& anim->HaveAnimationsFor( mat ) )
	{
		bindedMaterials.push_back( mat );
		AnimationLink *currentLink = mat->animationLink;
		if( currentLink )
		{
			currentLink->UnbindMaterial( mat );
		}
		mat->animationLink = this;
		anim->LoadKeyFrames( mat );
		mat->SetFrame( currentFrame );
		return true;
	}
	return false;
}

void AnimationLink::UnbindMaterial( Material *mat )
{
	std::vector< Material * >::iterator it = FindMaterial( mat );
	if( it != bindedMaterials.end() )
	{
		bindedMaterials.erase( it );
		mat->animationLink = NULL;
	}
}

void AnimationLink::SetFrame( FrameNumber num )
{
	//gprintf( "AnimationLink::SetFrame( %f )  %u, %u\n", num, bindedPanes.size(), bindedMaterials.size() );
	currentFrame = num;
	u32 cnt = bindedPanes.size();
	for( u32 i = 0; i < cnt; i++ )
	{
		bindedPanes[ i ]->SetFrame( num );
	}

	cnt = bindedMaterials.size();
	for( u32 i = 0; i < cnt; i++ )
	{
		bindedMaterials[ i ]->SetFrame( num );
	}
}

void AnimationLink::AdvanceFrame()
{
	if( animDone )
	{
		return;
	}
	if( direction == 1 && currentFrame < endFrame )
	{
		currentFrame++;
	}
	else if( direction == -1 && currentFrame > startFrame )
	{
		currentFrame--;
	}
	else if( loop == LoopNo )
	{
		animDone = true;
		currentFrame = waitFrame;
	}
	else
	{
		if( forwardAndBackward )
		{
			direction *= -1;
			currentFrame += direction;
		}
		else
		{
			currentFrame = startFrame;
		}
	}

	SetFrame( currentFrame );
}

std::vector< Pane * >::iterator AnimationLink::FindPane( Pane *pane )
{
	u32 cnt = bindedPanes.size();
	for( u32 i = 0; i < cnt; i++ )
	{
		if( bindedPanes[ i ] == pane )
		{
			return bindedPanes.begin() + i;
		}
	}
	return bindedPanes.end();
}

std::vector< Material * >::iterator AnimationLink::FindMaterial( Material *mat )
{
	u32 cnt = bindedMaterials.size();
	for( u32 i = 0; i < cnt; i++ )
	{
		if( bindedMaterials[ i ] == mat )
		{
			return bindedMaterials.begin() + i;
		}
	}
	return bindedMaterials.end();
}
