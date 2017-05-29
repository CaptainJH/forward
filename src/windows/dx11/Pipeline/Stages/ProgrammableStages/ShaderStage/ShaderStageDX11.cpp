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
#include "ShaderStageDX11.h"
#include "dx11/RendererDX11.h"
#include "dx11/ResourceSystem/Buffer/ConstantBufferDX11.h"
#include "dx11/ResourceSystem/ResourceView/ShaderResourceViewDX11.h"
#include "dx11/ResourceSystem/ResourceView/UnorderedAccessViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
static void SetToNull( void* pArray, i32 num )
{
    const i32 ptrSize = sizeof( void* );
    memset( pArray, 0, num * ptrSize );
}
//--------------------------------------------------------------------------------
ShaderStageDX11::ShaderStageDX11()
{
	// Link the two states together to monitor their changes.
	DesiredState.SetSisterState( &CurrentState );
}
//--------------------------------------------------------------------------------
ShaderStageDX11::~ShaderStageDX11()
{
	
}
//--------------------------------------------------------------------------------
void ShaderStageDX11::SetFeatureLevel( D3D_FEATURE_LEVEL level )
{
	m_FeatureLevel = level;
}
//--------------------------------------------------------------------------------
void ShaderStageDX11::ClearDesiredState( )
{
	DesiredState.ClearState();
}
//--------------------------------------------------------------------------------
void ShaderStageDX11::ClearCurrentState( )
{
	CurrentState.ClearState();
}
//--------------------------------------------------------------------------------
void ShaderStageDX11::ApplyDesiredState( ID3D11DeviceContext* pContext )
{
	// TODO: remove the 'count' arguments from the Bind* methods, since the number
	//       is tracked internally by the state itself.

	// Compare the current shader vs. the desired shader and set it if necesary.
	if ( DesiredState.ShaderProgram.IsUpdateNeeded() ) {
		BindShaderProgram( pContext );
	}

	// Compare the constant buffer state and set it if necesary.
	if ( DesiredState.ConstantBuffers.IsUpdateNeeded() ) {
		BindConstantBuffers( pContext, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT-1 );
	}

	// Compare the sampler states and set them if necesary.
	if ( DesiredState.SamplerStates.IsUpdateNeeded() ) {
		BindSamplerStates( pContext, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT-1 );
	}

	// Compare the shader resource view states and set them if necesary.
	if ( DesiredState.ShaderResources.IsUpdateNeeded() ) {
		BindShaderResourceViews( pContext, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT-1 ); 
	}

	// Compare the unordered access view states and set them if necesary.
	if ( DesiredState.UnorderedAccessResources.IsUpdateNeeded() 
		|| DesiredState.UAVInitialCounts.IsUpdateNeeded() ) {
		if ( m_FeatureLevel != D3D_FEATURE_LEVEL_11_0 )
			BindUnorderedAccessViews( pContext, 1 );
		else
			BindUnorderedAccessViews( pContext, D3D11_PS_CS_UAV_REGISTER_COUNT-1 );
	}

	// After binding everything, set the current state to the desired state.
	
	DesiredState.ResetUpdateFlags();
	CurrentState = DesiredState;
}
//--------------------------------------------------------------------------------
bool ShaderStageDX11::extractConstantBuffers(const u32 length, ID3D11Buffer** pResult)
{
	for (u32 i = 0; i < length; ++i)
	{
		ResourcePtr ptr = DesiredState.ConstantBuffers.GetState(i);
		if (ptr == nullptr)
			continue;
		auto bufferID = ptr->m_iResource;
		ConstantBufferDX11* pBuffer = RendererDX11::Get()->GetConstantBufferByIndex(bufferID);
		pResult[i] = static_cast<ID3D11Buffer*>(pBuffer->GetResource());
	}

	return true;
}
//--------------------------------------------------------------------------------
bool ShaderStageDX11::extractShaderResourceViews(const u32 length, ID3D11ShaderResourceView** pResult)
{
	for (u32 i = 0; i < length; ++i)
	{
		ResourcePtr ptr = DesiredState.ShaderResources.GetState(i);
		if (ptr == nullptr)
			continue;
		auto shaderViewID = ptr->m_iResourceSRV;
		ShaderResourceViewDX11& srv = RendererDX11::Get()->GetShaderResourceViewByIndex(shaderViewID);
		pResult[i] = static_cast<ID3D11ShaderResourceView*>(srv.GetSRV());
	}

	return true;
}
//--------------------------------------------------------------------------------
bool ShaderStageDX11::extractUnorderAccessResourceViews(const u32 length, ID3D11UnorderedAccessView** pResult)
{
	for (u32 i = 0; i < length; ++i)
	{
		ResourcePtr ptr = DesiredState.UnorderedAccessResources.GetState(i);
		if (ptr == nullptr)
			continue;
		auto unorderViewID = ptr->m_iResourceUAV;
		UnorderedAccessViewDX11& uav = RendererDX11::Get()->GetUnorderedAccessViewByIndex(unorderViewID);
		pResult[i] = static_cast<ID3D11UnorderedAccessView*>(uav.GetUAV());
	}

	return true;
}