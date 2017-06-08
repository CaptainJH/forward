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
#include "PCH.h"
#include "ComputeShaderStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderProgram/ComputeShaderDX11.h"
#include "dx11/RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ComputeStageDX11::ComputeStageDX11()
{
}
//--------------------------------------------------------------------------------
ComputeStageDX11::~ComputeStageDX11()
{
}
//--------------------------------------------------------------------------------
ShaderType ComputeStageDX11::GetType()
{
	return( COMPUTE_SHADER );
}
//--------------------------------------------------------------------------------
void ComputeStageDX11::BindShaderProgram( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();
	ShaderDX* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11ComputeShader* pShader = 0;
	
	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<ComputeShaderDX11*>( pShaderDX11 )->m_pComputeShader;
	}

	pContext->CSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void ComputeStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ConstantBuffers.Size;
	std::array<ID3D11Buffer*, Length> constantBuffers = { nullptr };
	extractConstantBuffers(Length, &constantBuffers[0]);
	pContext->CSSetConstantBuffers( 
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		&constantBuffers[0] );
}
//--------------------------------------------------------------------------------
void ComputeStageDX11::BindSamplerStates( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->CSSetSamplers( 
		DesiredState.SamplerStates.GetStartSlot(),
		DesiredState.SamplerStates.GetRange(),
		DesiredState.SamplerStates.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void ComputeStageDX11::BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ShaderResources.Size;
	std::array<ID3D11ShaderResourceView*, Length> shaderResources = { nullptr };
	extractShaderResourceViews(Length, &shaderResources[0]);
	pContext->CSSetShaderResources( 
		DesiredState.ShaderResources.GetStartSlot(),
		DesiredState.ShaderResources.GetRange(),
		&shaderResources[0] );
}
//--------------------------------------------------------------------------------
void ComputeStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	// Here we need to get the start and end slots from both the UAV states and the 
	// UAV initial counts, and take the superset of those to ensure that all of the
	// UAV states are accounted for.

	u32 minStartSlot = 
		min( DesiredState.UnorderedAccessResources.GetStartSlot(),
		DesiredState.UAVInitialCounts.GetStartSlot() );

	u32 maxEndSlot =
		max( DesiredState.UnorderedAccessResources.GetEndSlot(),
		DesiredState.UAVInitialCounts.GetEndSlot() );

	const auto Length = DesiredState.UnorderedAccessResources.Size;
	std::array<ID3D11UnorderedAccessView*, Length> unorderAccessResources = { nullptr };
	extractUnorderAccessResourceViews(Length, &unorderAccessResources[0]);
	pContext->CSSetUnorderedAccessViews( 
		minStartSlot,
		maxEndSlot - minStartSlot + 1,
		&unorderAccessResources[0],
		DesiredState.UAVInitialCounts.GetSlotLocation( minStartSlot ) );
}
//--------------------------------------------------------------------------------


