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
ShaderDX::ShaderDX(forward::FrameGraphObject* obj)
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

	s << L"[" << FileName << L"][" << Function << L"][" << ShaderModel << L"]";

	return( s.str() );
}
//--------------------------------------------------------------------------------
ID3DBlob* ShaderDX::GetCompiledCode()
{
	return m_pCompiledShader;
}