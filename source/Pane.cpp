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

#include "Pane.h"
#include "Layout.h"
#include "sc.h"
#include "video.h"

void Pane::Load(Pane::Header *pan)
{
	this->header = pan;
	this->hide = false;
	RootPane = !strcmp( header->name, "RootPane" );
}

Pane::~Pane()
{
	// delete children
	for(u32 i = 0; i < panes.size(); ++i)
		delete panes[i];
}

void Pane::SetFrame(FrameNumber frame)
{
	// setframe on self
	Animator::SetFrame( frame );

	// setframe on children
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->SetFrame( frame );
}

void Pane::Render(const Resources& resources, u8 parent_alpha, Mtx &modelview,
				  bool widescreen, bool modify_alpha) const
{
	if (!GetVisible() || GetHide())
		return;

	u8 render_alpha = header->alpha;

    if(RootPane && parent_alpha != 0xFF)
    {
        render_alpha = MultiplyAlpha(header->alpha, parent_alpha);
        modify_alpha = true;
    }
	else if(!RootPane && modify_alpha)
	{
		render_alpha = MultiplyAlpha(header->alpha, parent_alpha);
	}
	else if(GetInfluencedAlpha() && header->alpha != 0xff)
	{
		modify_alpha = true;
		parent_alpha = MultiplyAlpha(header->alpha, parent_alpha);
	}

	float ws_scale = 1.0f;

	if( widescreen && GetWidescren() )
	{
		ws_scale *= 0.82f; // should actually be 0.75?
		widescreen = false;
	}

	Mtx m1,m2,m3,m4;
	guMtxIdentity (m1);

	// Scale
	guMtxScaleApply(m1,m1, header->scale.x * ws_scale, header->scale.y, 1.f);

	// Rotate
	guMtxRotDeg ( m2, 'x', header->rotate.x );
	guMtxRotDeg ( m3, 'y', header->rotate.y );
	guMtxRotDeg ( m4, 'z', header->rotate.z );
	guMtxConcat(m2, m3, m2);
	guMtxConcat(m2, m4, m2);
	guMtxConcat(m1, m2, m1);

	// Translate
	guMtxTransApply(m1,m1, header->translate.x, header->translate.y, header->translate.z);

	guMtxConcat (modelview, m1, pane_view);

	bool scissor = gxScissorForBindedLayouts;
	u32 scissorX = 0;
	u32 scissorY = 0;
	u32 scissorW = 0;
	u32 scissorH = 0;

	// calculate scissors if they will be used
	if( scissor )
	{
		Mtx mv2, mv3;
		guMtxIdentity (mv2);
		guMtxIdentity (mv3);
		guMtxTransApply(mv2,mv2, -0.5f * GetOriginX() * GetWidth(), -0.5f * GetOriginY() * GetHeight(), 0.f);
		guMtxTransApply(mv3,mv3, 0.5f * GetOriginX() * GetWidth(), 0.5f * GetOriginY() * GetHeight(), 0.f);
		guMtxScaleApply(mv2, mv2, 1.0f, -1.0f, 1.0f);
		guMtxScaleApply(mv3, mv3, 1.0f, -1.0f, 1.0f);
		guMtxConcat (pane_view, mv2, mv2);
		guMtxConcat (pane_view, mv3, mv3);

		f32 viewport[6];
		f32 projection[7];

		GX_GetViewportv(viewport);
        GX_GetProjectionv(MainProjection, projection, GX_ORTHOGRAPHIC);

		guVector vecTL;
		guVector vecBR;
		GX_Project(0.0f, 0.0f, 0.0f, mv2, projection, viewport, &vecTL.x, &vecTL.y, &vecTL.z);
		GX_Project(0.0f, 0.0f, 0.0f, mv3, projection, viewport, &vecBR.x, &vecBR.y, &vecBR.z);

        // round up scissor box offset and round down the size
        scissorX = (u32)(0.5f + std::max(vecTL.x, 0.0f));
        scissorY = (u32)(0.5f + std::max(vecTL.y, 0.0f));
        scissorW = (u32)std::max(vecBR.x - vecTL.x, 0.0f);
        scissorH = (u32)std::max(vecBR.y - vecTL.y, 0.0f);

		GX_SetScissor( scissorX, scissorY, scissorW, scissorH );
	}

	// binded layouts dont inheiret the modified widescreen setting
	bool realWS = ( _CONF_GetAspectRatio() == CONF_ASPECT_16_9 );

	// draw binded layouts that appear under this one
	foreach( Layout *l, bindedLayoutsUnder )
	{
		l->RenderWithCurrentMtx( pane_view, realWS );
	}
	if( scissor )
	{
		GX_SetScissor( 0, 0, screenwidth, screenheight );
	}

	// render self
	Draw(resources, render_alpha, ws_scale, pane_view);

	// render children
	for(u32 i = 0; i < panes.size(); ++i)
		panes[i]->Render(resources, render_alpha, pane_view, widescreen, modify_alpha);

	// draw binded panes that appear on top of this one
	if( scissor )
	{
		GX_SetScissor( scissorX, scissorY, scissorW, scissorH );
	}
	foreach( Layout *l, bindedLayoutsOver )
	{
		l->RenderWithCurrentMtx( pane_view, realWS );
	}
	if( scissor )
	{
		GX_SetScissor( 0, 0, screenwidth, screenheight );
	}
}

Pane* Pane::FindPane(const std::string& find_name)
{
	if (find_name.compare(0, 0x10, getName()) == 0)
		return this;

	for(u32 i = 0; i < panes.size(); ++i)
	{
		Pane *found = panes[i]->FindPane(find_name);
		if (found)
			return found;
	}

	return NULL;
}

Pane* Pane::FindPane( const char* find_name )
{
	if( !strncmp( find_name, getName(), 0x10 ) )
	{
		return this;
	}

	for(u32 i = 0; i < panes.size(); ++i)
	{
		Pane *found = panes[i]->FindPane( find_name );
		if (found)
			return found;
	}

	return NULL;
}

void Pane::BindLayout( Layout *layout, bool under )
{
	if( under )
	{
		bindedLayoutsUnder << layout;
	}
	else
	{
		bindedLayoutsOver << layout;
	}
}

void Pane::UnbindAllLayouts()
{
	bindedLayoutsUnder.clear();
	bindedLayoutsOver.clear();
}

void Pane::ProcessHermiteKey(const KeyType& type, float value)
{
	if (type.type == ANIMATION_TYPE_VERTEX_COLOR)	// vertex color
	{
		// only alpha is supported for Panes afaict
		if (0x10 == type.target)
		{
			header->alpha = FLOAT_2_U8(value);
			return;
		}
	}
	else if (type.type == ANIMATION_TYPE_PANE)	// pane animation
	{
		if (type.target < 10)
		{
			(&header->translate.x)[type.target] = value;
			return;
		}
	}

	Base::ProcessHermiteKey(type, value);
}

void Pane::ProcessStepKey(const KeyType& type, StepKeyHandler::KeyData data)
{
	if (type.type == ANIMATION_TYPE_VISIBILITY)	// visibility
	{
		SetVisible(!!data.data2);
		return;
	}

	Base::ProcessStepKey(type, data);
}
