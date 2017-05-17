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
#include "DomainShaderStageDX11.h"
#include "Pipeline\Stages\ProgrammableStages\ShaderProgram\DomainShaderDX11.h"
#include "RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
DomainStageDX11::DomainStageDX11()
{
}
//--------------------------------------------------------------------------------
DomainStageDX11::~DomainStageDX11()
{
}
//--------------------------------------------------------------------------------
ShaderType DomainStageDX11::GetType()
{
	return( DOMAIN_SHADER );
}
//--------------------------------------------------------------------------------
void DomainStageDX11::BindShaderProgram( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();
	ShaderDX11* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11DomainShader* pShader = 0;
	
	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<DomainShaderDX11*>( pShaderDX11 )->m_pDomainShader;
	}

	pContext->DSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void DomainStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ConstantBuffers.Size;
	std::array<ID3D11Buffer*, Length> constantBuffers = { nullptr };
	extractConstantBuffers(Length, &constantBuffers[0]);
	pContext->DSSetConstantBuffers( 
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		&constantBuffers[0] );
}
//--------------------------------------------------------------------------------
void DomainStageDX11::BindSamplerStates( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->DSSetSamplers( 
		DesiredState.SamplerStates.GetStartSlot(),
		DesiredState.SamplerStates.GetRange(),
		DesiredState.SamplerStates.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void DomainStageDX11::BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ShaderResources.Size;
	std::array<ID3D11ShaderResourceView*, Length> shaderResources = { nullptr };
	extractShaderResourceViews(Length, &shaderResources[0]);
	pContext->DSSetShaderResources( 
		DesiredState.ShaderResources.GetStartSlot(),
		DesiredState.ShaderResources.GetRange(),
		&shaderResources[0] ); 
}
//--------------------------------------------------------------------------------
void DomainStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* /*pContext*/, i32 /*count*/ )
{
	// Do nothing - the geometry shader doesn't support UAV's!
}
//--------------------------------------------------------------------------------

