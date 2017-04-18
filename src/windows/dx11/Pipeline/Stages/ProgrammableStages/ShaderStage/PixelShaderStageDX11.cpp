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
#include "PixelShaderStageDX11.h"
#include "Pipeline\Stages\ProgrammableStages\ShaderProgram\PixelShaderDX11.h"
#include "RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
PixelStageDX11::PixelStageDX11()
{
}
//--------------------------------------------------------------------------------
PixelStageDX11::~PixelStageDX11()
{
}
//--------------------------------------------------------------------------------
ShaderType PixelStageDX11::GetType()
{
	return( PIXEL_SHADER );
}
//--------------------------------------------------------------------------------
void PixelStageDX11::BindShaderProgram( ID3D11DeviceContext* pContext )
{
	RendererDX11* pRenderer = RendererDX11::Get();
	ShaderDX11* pShaderDX11 = pRenderer->GetShader( DesiredState.ShaderProgram.GetState() );

	ID3D11PixelShader* pShader = 0;

	if ( pShaderDX11 ) {
		pShader = reinterpret_cast<PixelShaderDX11*>( pShaderDX11 )->m_pPixelShader;
	}

	pContext->PSSetShader( pShader, 0, 0 );
}
//--------------------------------------------------------------------------------
void PixelStageDX11::BindConstantBuffers( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->PSSetConstantBuffers(
		DesiredState.ConstantBuffers.GetStartSlot(),
		DesiredState.ConstantBuffers.GetRange(),
		DesiredState.ConstantBuffers.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void PixelStageDX11::BindSamplerStates( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->PSSetSamplers( 
		DesiredState.SamplerStates.GetStartSlot(),
		DesiredState.SamplerStates.GetRange(),
		DesiredState.SamplerStates.GetFirstSlotLocation() );
}
//--------------------------------------------------------------------------------
void PixelStageDX11::BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 /*count*/ )
{
	pContext->PSSetShaderResources( 
		DesiredState.ShaderResourceViews.GetStartSlot(),
		DesiredState.ShaderResourceViews.GetRange(),
		DesiredState.ShaderResourceViews.GetFirstSlotLocation() ); 
}
//--------------------------------------------------------------------------------
void PixelStageDX11::BindUnorderedAccessViews( ID3D11DeviceContext* /*pContext*/, i32 /*count*/ )
{
	// Do nothing - the pixel shader supports UAV's, but the API isn't clear on how to!
}
//--------------------------------------------------------------------------------

