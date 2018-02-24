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
#include "OutputMergerStageStateDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
OutputMergerStageStateDX11::OutputMergerStageStateDX11() :
	BlendState( -1 ),
	DepthStencilState( -1 ),
	StencilRef( 0 ),
	RenderTargetResources( 0 ),
	DepthTargetResources( 0 ),
	UnorderedAccessResources( 0 ),
	UAVInitialCounts( 0 ),
	m_FeatureLevel( D3D_FEATURE_LEVEL_9_1 ),
	m_pSisterState( nullptr )
{
	ClearState();
}
//--------------------------------------------------------------------------------
OutputMergerStageStateDX11::~OutputMergerStageStateDX11()
{
	
}
//--------------------------------------------------------------------------------
void OutputMergerStageStateDX11::SetFeautureLevel( D3D_FEATURE_LEVEL level )
{
	m_FeatureLevel = level;
}
//--------------------------------------------------------------------------------
i32 OutputMergerStageStateDX11::GetRenderTargetCount() const
{
	u32 count = 0;

	for ( u32 i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++ )
	{
		if ( RenderTargetResources.GetState( i ) != nullptr )
			count++;
	}

	return( count );
}
//--------------------------------------------------------------------------------
void OutputMergerStageStateDX11::ClearState( )
{
	BlendState.InitializeState();
	DepthStencilState.InitializeState();
	StencilRef.InitializeState();
	RenderTargetResources.InitializeStates();
	DepthTargetResources.InitializeState();
	UnorderedAccessResources.InitializeStates();
	UAVInitialCounts.InitializeStates();
}
//--------------------------------------------------------------------------------
void OutputMergerStageStateDX11::SetSisterState( OutputMergerStageStateDX11* pState )
{
	m_pSisterState = pState;

	BlendState.SetSister( &m_pSisterState->BlendState );
	DepthStencilState.SetSister( &m_pSisterState->DepthStencilState );
	StencilRef.SetSister( &m_pSisterState->StencilRef );
	RenderTargetResources.SetSister( &m_pSisterState->RenderTargetResources );
	DepthTargetResources.SetSister( &m_pSisterState->DepthTargetResources );
	UnorderedAccessResources.SetSister( &m_pSisterState->UnorderedAccessResources );
	UAVInitialCounts.SetSister( &m_pSisterState->UAVInitialCounts );
}
//--------------------------------------------------------------------------------
void OutputMergerStageStateDX11::ResetUpdateFlags( )
{
	BlendState.ResetTracking();
	DepthStencilState.ResetTracking();
	StencilRef.ResetTracking();
	RenderTargetResources.ResetTracking();
	DepthTargetResources.ResetTracking();
	UnorderedAccessResources.ResetTracking();
	UAVInitialCounts.ResetTracking();
}
//--------------------------------------------------------------------------------
