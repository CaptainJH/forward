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
#include "ShaderDX.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ShaderDX::ShaderDX(forward::GraphicsObject* obj)
	: DeviceObject(obj)
	, FileName()
	, Function()
	, ShaderModel()
	, ShaderText()
	, m_pCompiledShader(nullptr)
{
}

ShaderDX::ShaderDX() 
	: DeviceObject(nullptr)
	, FileName()
	, Function()
	, ShaderModel()
	, ShaderText()
	, m_pCompiledShader( nullptr )
{
}
//--------------------------------------------------------------------------------
ShaderDX::~ShaderDX()
{
	SAFE_RELEASE( m_pCompiledShader );
	//SAFE_DELETE( m_pReflection );
}
//--------------------------------------------------------------------------------
//void ShaderDX11::SetReflection( ShaderReflectionDX11* pReflection )
//{
//	m_pReflection = pReflection;
//}
//--------------------------------------------------------------------------------
//ShaderReflectionDX11* ShaderDX11::GetReflection( )
//{
//	return( m_pReflection );
//}
//--------------------------------------------------------------------------------
std::wstring ShaderDX::ToString()
{
	std::wstringstream s;

	s << L"[" << FileName << L"][" << Function << L"][" << TextHelper::ToUnicode(ShaderModel) << L"]";

	return( s.str() );
}
//--------------------------------------------------------------------------------
void* ShaderDX::GetCompiledCode()
{
	if (m_CompiledShader6.empty())
		return m_pCompiledShader->GetBufferPointer();
	else
		return m_CompiledShader6.data();
}

u64 ShaderDX::GetCompiledCodeSize() const
{
	if (m_CompiledShader6.empty())
		return m_pCompiledShader->GetBufferSize();
	else
		return m_CompiledShader6.size();
}

std::vector<HLSLConstantBuffer> const& ShaderDX::GetCBuffers() const
{
	return m_CBuffers;
}

std::vector<HLSLTextureBuffer> const& ShaderDX::GetTBuffers() const
{
	return m_TBuffers;
}

std::vector<HLSLTexture> const& ShaderDX::GetTextures() const
{
	return m_Textures;
}

std::vector<HLSLTextureArray> const& ShaderDX::GetTextureArrays() const
{
	return m_TextureArrays;
}

std::vector<HLSLByteAddressBuffer> const& ShaderDX::GetByteAddressBuffers() const
{
	return m_RBuffers;
}

std::vector<HLSLStructuredBuffer> const& ShaderDX::GetStructuredBuffers() const
{
	return m_SBuffers;
}

void ShaderDX::InsertInput(HLSLParameter const& parameter)
{
	m_inputs.push_back(parameter);
}

void ShaderDX::InsertOutput(HLSLParameter const& parameter)
{
	m_outputs.push_back(parameter);
}

void ShaderDX::Insert(HLSLConstantBuffer const& cbuffer)
{
	if (std::find_if(m_CBuffers.begin(), m_CBuffers.end(), [&](auto& cb)->bool {
		return cb.GetName() == cbuffer.GetName();
		}) == m_CBuffers.end())
		m_CBuffers.push_back(cbuffer);
}

void ShaderDX::Insert(HLSLResourceBindInfo const& rbinfo)
{
	if (std::find_if(m_RBInfos.begin(), m_RBInfos.end(), [&](auto& rb)->bool {
		return rb.GetName() == rbinfo.GetName();
		}) == m_RBInfos.end())
		m_RBInfos.push_back(rbinfo);
}

void ShaderDX::Insert(HLSLTextureBuffer const& tbuffer)
{
	m_TBuffers.push_back(tbuffer);
}

void ShaderDX::Insert(HLSLTexture const& texture)
{
	if (std::find_if(m_Textures.begin(), m_Textures.end(), [&](auto& t)->bool {
		return texture.GetName() == t.GetName();
		}) == m_Textures.end())
		m_Textures.push_back(texture);
}

void ShaderDX::Insert(HLSLTextureArray const& tarray)
{
	m_TextureArrays.push_back(tarray);
}

void ShaderDX::Insert(HLSLSamplerState const& samp)
{
	if (std::find_if(m_Samplers.begin(), m_Samplers.end(), [&](auto& s)->bool {
		return samp.GetName() == s.GetName();
		}) == m_Samplers.end())
		m_Samplers.push_back(samp);
}

void ShaderDX::Insert(HLSLByteAddressBuffer const& rbuffer)
{
	if (std::find_if(m_RBuffers.begin(), m_RBuffers.end(), [&](auto& rb)->bool {
		return rb.GetName() == rbuffer.GetName();
		}) == m_RBuffers.end())
		m_RBuffers.push_back(rbuffer);
}

void ShaderDX::Insert(HLSLStructuredBuffer const& sbuffer)
{
	if (std::find_if(m_SBuffers.begin(), m_SBuffers.end(), [&](auto& sb)->bool {
		return sb.GetName() == sbuffer.GetName();
		}) == m_SBuffers.end())
		m_SBuffers.push_back(sbuffer);
}