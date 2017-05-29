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
#include "VertexShaderStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderProgram/VertexShaderDX11.h"
#include "dx11/RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
VertexStageDX11::VertexStageDX11()
{
}
//--------------------------------------------------------------------------------
VertexStageDX11::~VertexStageDX11()
{
}
//--------------------------------------------------------------------------------
ShaderType VertexStageDX11::GetType()
{
	return( VERTEX_SHADER );
}
//--------------------------------------------------------------------------------
void VertexStageDX11::BindShaderProgram( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();
	ShaderDX11* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11VertexShader* pShader = 0;
		
	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<VertexShaderDX11*>( pShaderDX11 )->m_pVertexShader;
	}

	pContext->VSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void VertexStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ConstantBuffers.Size;
	std::array<ID3D11Buffer*, Length> constantBuffers = { nullptr };
	extractConstantBuffers(Length, &constantBuffers[0]);
	pContext->VSSetConstantBuffers( 
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		&constantBuffers[0] );
}
//--------------------------------------------------------------------------------
void VertexStageDX11::BindSamplerStates( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->VSSetSamplers( 
		DesiredState.SamplerStates.GetStartSlot(),
		DesiredState.SamplerStates.GetRange(),
		DesiredState.SamplerStates.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void VertexStageDX11::BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ShaderResources.Size;
	std::array<ID3D11ShaderResourceView*, Length> shaderResources = { nullptr };
	extractShaderResourceViews(Length, &shaderResources[0]);
	pContext->VSSetShaderResources( 
		DesiredState.ShaderResources.GetStartSlot(),
		DesiredState.ShaderResources.GetRange(),
		&shaderResources[0] ); 
}
//--------------------------------------------------------------------------------
void VertexStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* /*pContext*/, i32 /*count*/ )
{
	// Do nothing - the vertex shader doesn't support UAV's!
}
//--------------------------------------------------------------------------------
