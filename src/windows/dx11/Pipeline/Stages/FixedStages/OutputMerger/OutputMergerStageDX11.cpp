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
#include "OutputMergerStageDX11.h"
#include "dx11/ResourceSystem/ResourceView/RenderTargetViewDX11.h"
#include "dx11/ResourceSystem/ResourceView/DepthStencilViewDX11.h"
#include "dx11/ResourceSystem/ResourceView/UnorderedAccessViewDX11.h"
#include "dx11/RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
OutputMergerStageDX11::OutputMergerStageDX11()
{
	DesiredState.SetSisterState( &CurrentState );
}
//--------------------------------------------------------------------------------
OutputMergerStageDX11::~OutputMergerStageDX11()
{
}
//--------------------------------------------------------------------------------
void OutputMergerStageDX11::SetFeautureLevel( D3D_FEATURE_LEVEL level )
{
	m_FeatureLevel = level;
	CurrentState.SetFeautureLevel( level );
	DesiredState.SetFeautureLevel( level );
}
//--------------------------------------------------------------------------------
void OutputMergerStageDX11::ClearDesiredState( )
{
	DesiredState.ClearState();
}
//--------------------------------------------------------------------------------
void OutputMergerStageDX11::ClearCurrentState( )
{
	CurrentState.ClearState();
}
//--------------------------------------------------------------------------------
void OutputMergerStageDX11::ApplyDesiredState( ID3D11DeviceContext* pContext )
{
	ApplyDesiredRenderTargetStates( pContext );
	ApplyDesiredBlendAndDepthStencilStates( pContext );
}
//--------------------------------------------------------------------------------
void OutputMergerStageDX11::ApplyDesiredRenderTargetStates( ID3D11DeviceContext* pContext )
{
	i32 rtvCount = 0;
	i32 uavCount = 0;

	if ( DesiredState.RenderTargetResources.IsUpdateNeeded() 
		|| DesiredState.UnorderedAccessResources.IsUpdateNeeded()
		|| DesiredState.DepthTargetResources.IsUpdateNeeded() ) 
	{
		RendererDX11* pRenderer = RendererDX11::Get();

		ID3D11RenderTargetView*	rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { 0 };
		ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { 0 };
		ID3D11DepthStencilView* dsv = 0;

		for ( i32 i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++ ) 
		{
			auto rtvPtr = DesiredState.RenderTargetResources.GetState(i);
			if (rtvPtr == nullptr)
				continue;
			RenderTargetViewDX11& rtv = pRenderer->GetRenderTargetViewByIndex( rtvPtr->m_iResourceRTV );
			rtvs[i] = rtv.m_pRenderTargetView.Get();

			if ( rtvs[i] != nullptr ) {
				rtvCount = i+1; // Record the number of non-null rtvs...
			}
		}

		for ( i32 i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++ ) 
		{
			ResourcePtr uavPtr = DesiredState.UnorderedAccessResources.GetState(i);
			if (uavPtr == nullptr)
				continue;
			UnorderedAccessViewDX11& uav = pRenderer->GetUnorderedAccessViewByIndex( uavPtr->m_iResourceUAV );
			uavs[i] = uav.m_pUnorderedAccessView.Get();

			if ( uavs[i] != nullptr ) {
				uavCount = i+1; // Record the number of non-null uavs...
			}
		}

		ResourcePtr dsvPtr = DesiredState.DepthTargetResources.GetState();
		if (dsvPtr != nullptr)
		{
			DepthStencilViewDX11& DSV = pRenderer->GetDepthStencilViewByIndex(dsvPtr->m_iResourceDSV);
			dsv = DSV.m_pDepthStencilView.Get();
		}

		// TODO: convert this to bind the UAVs too...
		pContext->OMSetRenderTargets( D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, dsv );
		//pContext->OMSetRenderTargetsAndUnorderedAccessViews( rtvCount, rtvs, dsv, 
		//	rtvCount, uavCount, uavs, (u32*)&DesiredState.UAVInitialCounts );

		// TODO: Find a better way to copy the state from desired to current...

		for ( i32 i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++ ) 
		{
			CurrentState.RenderTargetResources.SetState( i, DesiredState.RenderTargetResources.GetState( i ) );
		}

		for ( i32 i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++ ) 
		{
			CurrentState.UnorderedAccessResources.SetState( i, DesiredState.UnorderedAccessResources.GetState( i ) );
			CurrentState.UAVInitialCounts.SetState( i, DesiredState.UAVInitialCounts.GetState( i ) );
		}

		CurrentState.DepthTargetResources.SetState( DesiredState.DepthTargetResources.GetState() );

		DesiredState.RenderTargetResources.ResetTracking();
		DesiredState.UnorderedAccessResources.ResetTracking();
		DesiredState.UAVInitialCounts.ResetTracking();
		DesiredState.DepthTargetResources.ResetTracking();
	}
}
//--------------------------------------------------------------------------------
void OutputMergerStageDX11::ApplyDesiredBlendAndDepthStencilStates( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();

	if ( DesiredState.BlendState.IsUpdateNeeded() ) {

		BlendStateComPtr pGlyphBlendState = pRenderer->GetBlendState( DesiredState.BlendState.GetState() );

		if ( nullptr != pGlyphBlendState ) {
			
			ID3D11BlendState* pBlendState = pGlyphBlendState.Get();

			// TODO: Add in the blend factors as states to the OutputMergerStageStateDX11 class!
			if ( pBlendState ) {
				f32 afBlendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				pContext->OMSetBlendState( pBlendState, afBlendFactors, 0xFFFFFFFF );
			}

			CurrentState.BlendState.SetState( DesiredState.BlendState.GetState() );
			DesiredState.BlendState.ResetTracking();
		}
	}

	if ( DesiredState.DepthStencilState.IsUpdateNeeded() || DesiredState.StencilRef.IsUpdateNeeded() ) {

		DepthStencilStateComPtr pGlyphDepthStencilState = pRenderer->GetDepthState( DesiredState.DepthStencilState.GetState() );
		
		if ( nullptr != pGlyphDepthStencilState ) {

			ID3D11DepthStencilState* pDepthState = pGlyphDepthStencilState.Get();

			if ( pDepthState ) {
				pContext->OMSetDepthStencilState( pDepthState, DesiredState.StencilRef.GetState() );
			}

			CurrentState.DepthStencilState.SetState( DesiredState.DepthStencilState.GetState() );
			CurrentState.StencilRef.SetState( DesiredState.StencilRef.GetState() );
			DesiredState.DepthStencilState.ResetTracking();
			DesiredState.StencilRef.ResetTracking();
		}
	}
}
//--------------------------------------------------------------------------------
const OutputMergerStageStateDX11& OutputMergerStageDX11::GetCurrentState() const
{
	return( CurrentState );
}
//--------------------------------------------------------------------------------
