//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) Jason Zink 
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// RasterizerStageStateDX11
//
//--------------------------------------------------------------------------------
#ifndef RasterizerStageStateDX11_h
#define RasterizerStageStateDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "TStateMonitor.h"
#include "TStateArrayMonitor.h"
#include "RendererCapability.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class RasterizerStageStateDX11
	{
	public:
		RasterizerStageStateDX11();
		virtual ~RasterizerStageStateDX11();

		void SetFeautureLevel( D3D_FEATURE_LEVEL level );
		void ClearState( );
		void SetSisterState( RasterizerStageStateDX11* pState );
		void ResetUpdateFlags( );

		TStateMonitor< i32 > RasterizerState;
		TStateMonitor< i32 > ViewportCount;
		TStateArrayMonitor< i32, FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE > Viewports;
		TStateMonitor< i32 > ScissorRectCount;
		TStateArrayMonitor< D3D11_RECT, FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE > ScissorRects;

	protected:

		D3D_FEATURE_LEVEL				m_FeatureLevel;

		RasterizerStageStateDX11*		m_pSisterState;
	};
};
//--------------------------------------------------------------------------------
#endif // RasterizerStageStateDX11_h
//--------------------------------------------------------------------------------

