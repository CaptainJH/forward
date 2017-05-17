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
#include "HullShaderStageDX11.h"
#include "Pipeline\Stages\ProgrammableStages\ShaderProgram\HullShaderDX11.h"
#include "RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
HullStageDX11::HullStageDX11()
{
}
//--------------------------------------------------------------------------------
HullStageDX11::~HullStageDX11()
{
}
//--------------------------------------------------------------------------------
ShaderType HullStageDX11::GetType()
{
	return( HULL_SHADER );
}
//--------------------------------------------------------------------------------
void HullStageDX11::BindShaderProgram( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();
	ShaderDX11* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11HullShader* pShader = 0;

	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<HullShaderDX11*>( pShaderDX11 )->m_pHullShader;
	}

	pContext->HSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void HullStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ConstantBuffers.Size;
	std::array<ID3D11Buffer*, Length> constantBuffers = { nullptr };
	extractConstantBuffers(Length, &constantBuffers[0]);
	pContext->HSSetConstantBuffers( 
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		&constantBuffers[0] );
}
//--------------------------------------------------------------------------------
void HullStageDX11::BindSamplerStates( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->HSSetSamplers( 
		DesiredState.SamplerStates.GetStartSlot(),
		DesiredState.SamplerStates.GetRange(),
		DesiredState.SamplerStates.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void HullStageDX11::BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ShaderResources.Size;
	std::array<ID3D11ShaderResourceView*, Length> shaderResources = { nullptr };
	extractShaderResourceViews(Length, &shaderResources[0]);
	pContext->HSSetShaderResources( 
		DesiredState.ShaderResources.GetStartSlot(),
		DesiredState.ShaderResources.GetRange(),
		&shaderResources[0] ); 
}
//--------------------------------------------------------------------------------
void HullStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* /*pContext*/, i32 /*count*/ )
{
	// Do nothing - the hull shader doesn't support UAV's!
}
//--------------------------------------------------------------------------------


