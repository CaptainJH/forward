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
#include "GeometryShaderStageDX11.h"
#include "dx11_Hieroglyph/Pipeline/Stages/ProgrammableStages/ShaderProgram/GeometryShaderDX11.h"
#include "dx11_Hieroglyph/RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
GeometryStageDX11::GeometryStageDX11()
{
}
//--------------------------------------------------------------------------------
GeometryStageDX11::~GeometryStageDX11()
{
}
//--------------------------------------------------------------------------------
ShaderType GeometryStageDX11::GetType()
{
	return( GEOMETRY_SHADER );
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindShaderProgram( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();
	ShaderDX* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11GeometryShader* pShader = 0;
	
	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<GeometryShaderDX11*>( pShaderDX11 )->m_pGeometryShader;
	}

	pContext->GSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ConstantBuffers.Size;
	std::array<ID3D11Buffer*, Length> constantBuffers = { nullptr };
	extractConstantBuffers(Length, &constantBuffers[0]);
	pContext->GSSetConstantBuffers( 
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		&constantBuffers[0] );
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindSamplerStates( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->GSSetSamplers( 
		DesiredState.SamplerStates.GetStartSlot(),
		DesiredState.SamplerStates.GetRange(),
		DesiredState.SamplerStates.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	const auto Length = DesiredState.ShaderResources.Size;
	std::array<ID3D11ShaderResourceView*, Length> shaderResources = { nullptr };
	extractShaderResourceViews(Length, &shaderResources[0]);
	pContext->GSSetShaderResources( 
		DesiredState.ShaderResources.GetStartSlot(),
		DesiredState.ShaderResources.GetRange(), 
		&shaderResources[0] ); 
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* /*pContext*/, i32 /*count*/ )
{
	// Do nothing - the geometry shader doesn't support UAV's!
}
//--------------------------------------------------------------------------------



