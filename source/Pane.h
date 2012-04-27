/*
Copyright (c) 2010 - Wii Banner Player Project
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

#ifndef WII_BNR_PANE_H_
#define WII_BNR_PANE_H_

#include <vector>
#include <gccore.h>

#include "Animator.h"
#include "list.h"

struct Resources;

class Pane;
typedef std::vector<Pane*> PaneList;

class Pane : public Animator
{
public:
	typedef Animator Base;
	Pane():gxScissorForBindedLayouts( false )
	{
	}

	static const u32 MAGIC = MAKE_FOURCC('p', 'a', 'n', '1');

	struct Header
	{
	  u32 magic;
	  u32 size_section;
	  u8 flags;
	  u8 origin;
	  u8 alpha;
	  u8 padding;
	  char name [0x10];
	  char user_data [0x08];
	  Vec3f translate;
	  Vec3f rotate;
	  Vec2f scale;
	  float width;
	  float height;
	} __attribute__((packed));

	void Load(Pane::Header *file);
	virtual ~Pane();

	const char *getName() const { return header->name; }

	void Render(const Resources& resources, u8 parent_alpha, Mtx &modelview,
				bool widescreen, bool modify_alpha = false) const;

	void SetFrame(FrameNumber frame );

	bool GetHide() const { return hide; }
	void SetHide(bool _hide) { hide = _hide; }

	bool GetVisible() const { return ((header->flags & (1 << FLAG_VISIBLE)) != 0); }
	void SetVisible(bool visible)
	{
		if(visible)
			header->flags |= (1 << FLAG_VISIBLE);
		else
			header->flags &= ~(1 << FLAG_VISIBLE);
	}

	u8 GetOriginX() const { return header->origin % 3; }
	u8 GetOriginY() const { return 2 - header->origin / 3; }

	float GetWidth() const { return header->width; }
	float GetHeight() const { return header->height; }

    Mtx& GetView() const { return pane_view; }

	bool GetInfluencedAlpha() const { return ((header->flags & (1 << FLAG_INFLUENCED_ALPHA)) != 0); }
	void SetInfluencedAlpha(bool influenced)
	{
		if(influenced)
			header->flags |= (1 << FLAG_INFLUENCED_ALPHA);
		else
			header->flags &= ~(1 << FLAG_INFLUENCED_ALPHA);
	}

	bool GetWidescren() const { return ((header->flags & (1 << FLAG_WIDESCREEN)) != 0); }

	Pane* FindPane(const std::string& name);	// recursive
	Pane* FindPane( const char* find_name );
	u8 GetAlpha() const { return header ? header->alpha : 0; }

	PaneList panes;

	virtual int GetMaterialIndex(){ return -1; }

	// stuff for binding layouts
	void BindLayout( Layout *layout, bool under );
	void UnbindAllLayouts();
	void SetGxScissorsForchildLayouts( bool scissors ){ gxScissorForBindedLayouts = scissors; }

	// added for the wiimote panes
	void SetPosition( f32 x, f32 y ){ if( header ){ header->translate.x = x; header->translate.y = y; } }
	void SetRotate( f32 degrees ){ if( header ){ header->rotate.z = degrees; } }

	void SetAlpha( u8 a ){ if( header ){ header->alpha = a; } }
	const f32 GetPosX() const { return header ? header->translate.x : 0; }
	const f32 GetPosY() const { return header ? header->translate.y : 0; }

	void SetRootPane( bool isRoot ){ RootPane = isRoot; }
	void SetScale( const Vec2f &v ){ if( header ){ header->scale = v; } }

protected:
	void ProcessHermiteKey(const KeyType& type, float value);
	void ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data);

private:
	virtual void Draw(const Resources&, u8, const float, Mtx&) const {}

	enum
	{
		FLAG_VISIBLE = 0x00,
		FLAG_INFLUENCED_ALPHA = 0x01,
		FLAG_WIDESCREEN = 0x02
	};

	Pane::Header *header;
	bool hide;	// used by the groups
	bool RootPane;
    mutable Mtx pane_view;  // view of pane on the last render execution


	// stuff for binding layouts onto panes so they move together as 1
	List< Layout* >bindedLayoutsUnder;
	List< Layout* >bindedLayoutsOver;

	// some binded panes need to be limited to the size of this pane, but not all of them
	bool gxScissorForBindedLayouts;
};

// apparently Bounding is just a regular pane
class Bounding : public Pane
{
public:
	static const u32 MAGIC = MAKE_FOURCC('b', 'n', 'd', '1');
};

#endif
