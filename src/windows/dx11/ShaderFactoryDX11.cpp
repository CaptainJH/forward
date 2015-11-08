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
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "ShaderFactoryDX11.h"
#include "Log.h"
#include "d3dUtil.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ShaderFactoryDX11::ShaderFactoryDX11( )
{
}
//--------------------------------------------------------------------------------
ShaderFactoryDX11::~ShaderFactoryDX11()
{
}
//--------------------------------------------------------------------------------
ID3DBlob* ShaderFactoryDX11::GenerateShader( ShaderType type, std::wstring& filename, std::wstring& function,
            std::wstring& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging )
{
	HRESULT hr = S_OK;

	std::wstringstream message;

	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrorMessages = nullptr;

	char AsciiFunction[1024];
	char AsciiModel[1024];
	WideCharToMultiByte(CP_ACP, 0, function.c_str(), -1, AsciiFunction, 1024, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, model.c_str(), -1, AsciiModel, 1024, NULL, NULL);

	// TODO: The compilation of shaders has to skip the warnings as errors 
	//       for the moment, since the new FXC.exe compiler in VS2012 is
	//       apparently more strict than before.

    UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; // | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	// Get the current path to the shader folders, and add the filename to it.

	std::wstring filepath = filename;

	// Load the file into memory

	std::ifstream shaderFile(filename);
	std::string hlslCode((std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());



	if ( FAILED( hr = D3DCompile( 
		hlslCode.c_str(),
		hlslCode.size(),
		nullptr,
		pDefines,
		nullptr,
		AsciiFunction,
		AsciiModel,
		flags,
		0,
		&pCompiledShader,
		&pErrorMessages ) ) )

	{
		message << L"Error compiling shader program: " << filepath << std::endl << std::endl;
		message << L"The following error was reported:" << std::endl;

		if ( ( enablelogging ) && ( pErrorMessages != nullptr ) )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			message << TextHelper::ToUnicode( std::string( pMessage ) );
			Log::Get().Write( message.str() );
		}

		SAFE_RELEASE( pCompiledShader );
		SAFE_RELEASE( pErrorMessages );

		return( nullptr );
	}

	SAFE_RELEASE( pErrorMessages );

	return( pCompiledShader );
}
//--------------------------------------------------------------------------------
ID3DBlob* ShaderFactoryDX11::GeneratePrecompiledShader( std::wstring& filename, std::wstring& function,
            std::wstring& model )
{
	// Create a blob to store the object code in
	
	ID3DBlob* pBlob = nullptr;


	// The file object will automatically be released when it goes out of scope,
	// and hence will free its loaded contents automatically also.

	return( pBlob );
}
//--------------------------------------------------------------------------------