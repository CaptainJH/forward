//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) Jason Zink 
//--------------------------------------------------------------------------------
#pragma warning( disable : 4244 )
#pragma warning( disable : 4239 )
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "ShaderFactoryDX.h"
#include "Log.h"
#include "utilities/Utils.h"
#include "dxCommon/d3dUtil.h"
#include "FileSystem.h"
#include "FileLoader.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ShaderFactoryDX::ShaderFactoryDX( )
{
}
//--------------------------------------------------------------------------------
ShaderFactoryDX::~ShaderFactoryDX()
{
}
//--------------------------------------------------------------------------------
ID3DBlob* ShaderFactoryDX::GenerateShader( ShaderType /*type*/, const std::string& shaderText, const std::string& function,
            const std::string& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging )
{
	HRESULT hr = S_OK;

	std::wstringstream message;

	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrorMessages = nullptr;

	// TODO: The compilation of shaders has to skip the warnings as errors 
	//       for the moment, since the new FXC.exe compiler in VS2012 is
	//       apparently more strict than before.

    u32 flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; // | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	if ( FAILED( hr = D3DCompile( 
		shaderText.c_str(),
		shaderText.length(),
		nullptr,
		pDefines,
		nullptr,
		function.c_str(),
		model.c_str(),
		flags,
		0,
		&pCompiledShader,
		&pErrorMessages ) ) )

	{
		message << L"Error compiling shader program" << std::endl;
		message << L"The following error was reported:" << std::endl;

		if ( ( enablelogging ) && ( pErrorMessages != nullptr ) )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			message << TextHelper::ToUnicode( std::string( pMessage ) );
			auto str = message.str();
			Log::Get().Write(str);
		}

		SAFE_RELEASE( pCompiledShader );
		SAFE_RELEASE( pErrorMessages );

		return( nullptr );
	}

	SAFE_RELEASE( pErrorMessages );

	return( pCompiledShader );
}
//--------------------------------------------------------------------------------