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
ID3DBlob* ShaderDX::GetCompiledCode()
{
	return m_pCompiledShader;
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
	m_CBuffers.push_back(cbuffer);
}

void ShaderDX::Insert(HLSLResourceBindInfo const& rbinfo)
{
	m_RBInfos.push_back(rbinfo);
}

void ShaderDX::Insert(HLSLTextureBuffer const& tbuffer)
{
	m_TBuffers.push_back(tbuffer);
}

void ShaderDX::Insert(HLSLTexture const& texture)
{
	m_Textures.push_back(texture);
}

void ShaderDX::Insert(HLSLTextureArray const& tarray)
{
	m_TextureArrays.push_back(tarray);
}

void ShaderDX::Insert(HLSLSamplerState const& samp)
{
	m_Samplers.push_back(samp);
}