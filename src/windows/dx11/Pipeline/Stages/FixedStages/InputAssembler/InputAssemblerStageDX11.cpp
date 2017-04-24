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
//#include "RendererDX11.h"
#include "InputAssemblerStageDX11.h"
#include "ResourceSystem\Buffer\VertexBufferDX11.h"
#include "ResourceSystem\Buffer\IndexBufferDX11.h"
#include "RendererDX11.h"
#include "Log.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
InputAssemblerStageDX11::InputAssemblerStageDX11()
{
	DesiredState.SetSisterState( &CurrentState );
}
//--------------------------------------------------------------------------------
InputAssemblerStageDX11::~InputAssemblerStageDX11()
{
	
}
//--------------------------------------------------------------------------------
void InputAssemblerStageDX11::SetFeautureLevel( D3D_FEATURE_LEVEL level )
{
	m_FeatureLevel = level;
	CurrentState.SetFeautureLevel( level );
	DesiredState.SetFeautureLevel( level );
}
//--------------------------------------------------------------------------------
void InputAssemblerStageDX11::ClearDesiredState()
{
	DesiredState.ClearState();
}
//--------------------------------------------------------------------------------
void InputAssemblerStageDX11::ClearCurrentState()
{
	CurrentState.ClearState();
}
//--------------------------------------------------------------------------------
void InputAssemblerStageDX11::ApplyDesiredState( ID3D11DeviceContext* pContext )
{
	// Bind the input layout first.

	RendererDX11* pRenderer = RendererDX11::Get();

	// Compare the primitive topology of the desired and current states
	if ( DesiredState.InputLayout.IsUpdateNeeded() ) 
	{
		InputLayoutComPtr pLayout = pRenderer->GetInputLayout( DesiredState.InputLayout.GetState() );

		if ( pLayout ) {
			pContext->IASetInputLayout( pLayout.Get() );
		} else {
			Log::Get().Write( L"Tried to bind an invalid input layout ID!" );
		}
	}

	// Bind the primitive topology
	if ( DesiredState.PrimitiveTopology.IsUpdateNeeded() ) 
	{
		pContext->IASetPrimitiveTopology( static_cast<D3D11_PRIMITIVE_TOPOLOGY>(DesiredState.PrimitiveTopology.GetState()) );
	}

	// Bind the vertex buffers
	if ( DesiredState.VertexBuffers.IsUpdateNeeded()
		|| DesiredState.VertexBufferOffsets.IsUpdateNeeded()
		|| DesiredState.VertexBufferStrides.IsUpdateNeeded() )
	{
		ID3D11Buffer* Buffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { NULL };

		for ( u32 i = 0; i < DesiredState.GetAvailableSlotCount(); i++ )
		{
			i32 index = DesiredState.VertexBuffers.GetState( i );

			VertexBufferDX11* pBuffer = pRenderer->GetVertexBufferByIndex( index );

			if ( pBuffer ) {
				Buffers[i] = static_cast<ID3D11Buffer*>( pBuffer->GetResource() );
			} else {
				Buffers[i] = 0;
			}
		}

		u32 startSlot = min( DesiredState.VertexBuffers.GetStartSlot(),
			min( DesiredState.VertexBufferOffsets.GetStartSlot(),
			DesiredState.VertexBufferStrides.GetStartSlot() ) );

		u32 endSlot = max( DesiredState.VertexBuffers.GetEndSlot(),
			max( DesiredState.VertexBufferOffsets.GetEndSlot(),
			DesiredState.VertexBufferStrides.GetEndSlot() ) );

		pContext->IASetVertexBuffers( 
			startSlot, endSlot-startSlot+1, 
			&Buffers[ startSlot ],
			DesiredState.VertexBufferStrides.GetSlotLocation( startSlot ),
			DesiredState.VertexBufferOffsets.GetSlotLocation( startSlot ) );
	}

	if ( DesiredState.IndexBuffer.IsUpdateNeeded() ) {
	
		i32 index = DesiredState.IndexBuffer.GetState();

		IndexBufferDX11* pBuffer = pRenderer->GetIndexBufferByIndex( index );

		DXGI_FORMAT dataFormat = static_cast<DXGI_FORMAT>(DesiredState.IndexBufferFormat.GetState());
		if ( pBuffer ) 
		{
			ID3D11Buffer* pIndexBuffer = reinterpret_cast<ID3D11Buffer*>( pBuffer->GetResource() );
			pContext->IASetIndexBuffer( pIndexBuffer, dataFormat, 0 );
		} 
		else 
		{
			pContext->IASetIndexBuffer( 0, dataFormat, 0 );
		}
	}

	DesiredState.ResetUpdateFlags();
	CurrentState = DesiredState;
}
//--------------------------------------------------------------------------------
const InputAssemblerStateDX11& InputAssemblerStageDX11::GetCurrentState() const
{
	return( CurrentState );
}
//--------------------------------------------------------------------------------
