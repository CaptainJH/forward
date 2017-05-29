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
#include "dx11/RendererDX11.h"
#include "RasterizerStageDX11.h"
#include "dx11/ResourceSystem/StateObject/ViewPortDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
RasterizerStageDX11::RasterizerStageDX11()
{
	// Link the two states together to monitor their changes.
	DesiredState.SetSisterState( &CurrentState );
}
//--------------------------------------------------------------------------------
RasterizerStageDX11::~RasterizerStageDX11()
{
	
}
//--------------------------------------------------------------------------------
void RasterizerStageDX11::SetFeatureLevel( D3D_FEATURE_LEVEL level )
{
	m_FeatureLevel = level;
	CurrentState.SetFeautureLevel( level );
	DesiredState.SetFeautureLevel( level );
}
//--------------------------------------------------------------------------------
void RasterizerStageDX11::ClearDesiredState()
{
	DesiredState.ClearState();
}
//--------------------------------------------------------------------------------
void RasterizerStageDX11::ClearCurrentState()
{
	CurrentState.ClearState();
}
//--------------------------------------------------------------------------------
const RasterizerStageStateDX11& RasterizerStageDX11::GetCurrentState() const
{
	return( CurrentState );
}
//--------------------------------------------------------------------------------
void RasterizerStageDX11::ApplyDesiredState( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();

	// Compare the current state vs. the desired state and set it if necesary.
	if ( DesiredState.RasterizerState.IsUpdateNeeded() )
	{
		RasterizerStateComPtr pGlyphRasterizerState 
			= pRenderer->GetRasterizerState( DesiredState.RasterizerState.GetState() );
		
		if ( pGlyphRasterizerState != nullptr ) {
			ID3D11RasterizerState* pRasterizerState = pGlyphRasterizerState.Get();
			pContext->RSSetState( pRasterizerState );
		}
	}

	// Compare the viewport state and set it if necesary.
	//i32 viewports = 0;
	
	// TODO: The viewport state should be directly holding D3D11_VIEWPORT structs instead of
	//       integer references to them.  This would simplify the handling, and reduce the 
	//       number of objects that need to be managed by the renderer!  Then we could also
	//       directly use the array stored in the TStateArrayMonitor too, removing the need
	//       for this copying loop.

	if ( DesiredState.ViewportCount.IsUpdateNeeded() 
		|| DesiredState.Viewports.IsUpdateNeeded() ) {
		
		D3D11_VIEWPORT aViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		
		for ( i32 i = 0; i < DesiredState.ViewportCount.GetState(); i++ ) {

			const ViewPortDX11& Viewport = pRenderer->GetViewPort( DesiredState.Viewports.GetState( i ) );
			
			aViewports[i] = Viewport.m_ViewPort; 
		}

		pContext->RSSetViewports( DesiredState.ViewportCount.GetState(), aViewports );
	}


	// Compare the scissor rect state and set it if necesary.

	// TODO: Same applies to the scissor rect state.  This should directly be the state that
	//       is monitored by the template...
	
	if ( DesiredState.ScissorRectCount.IsUpdateNeeded() || DesiredState.ScissorRects.IsUpdateNeeded() ) {
		
		D3D11_RECT aRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		
		for ( i32 i = 0; i < DesiredState.ScissorRectCount.GetState(); i++ ) {
			aRects[i] = DesiredState.ScissorRects.GetState( i );
		}

		pContext->RSSetScissorRects( DesiredState.ScissorRectCount.GetState(), aRects );
	}

	DesiredState.ResetUpdateFlags();
	CurrentState = DesiredState;
}
//--------------------------------------------------------------------------------
