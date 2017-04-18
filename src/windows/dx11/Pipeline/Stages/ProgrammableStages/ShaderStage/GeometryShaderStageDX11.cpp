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
#include "Pipeline\Stages\ProgrammableStages\ShaderProgram\GeometryShaderDX11.h"
#include "RendererDX11.h"
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
	ShaderDX11* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11GeometryShader* pShader = 0;
	
	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<GeometryShaderDX11*>( pShaderDX11 )->m_pGeometryShader;
	}

	pContext->GSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->GSSetConstantBuffers( 
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		DesiredState.ConstantBuffers.GetFirstSlotLocation() );
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
	pContext->GSSetShaderResources( 
		DesiredState.ShaderResourceViews.GetStartSlot(),
		DesiredState.ShaderResourceViews.GetRange(), 
		DesiredState.ShaderResourceViews.GetFirstSlotLocation() ); 
}
//--------------------------------------------------------------------------------
void GeometryStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* /*pContext*/, i32 /*count*/ )
{
	// Do nothing - the geometry shader doesn't support UAV's!
}
//--------------------------------------------------------------------------------



