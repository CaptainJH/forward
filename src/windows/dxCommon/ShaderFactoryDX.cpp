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
ID3DBlob* ShaderFactoryDX::GenerateShader( ShaderType /*type*/, const std::wstring& filename, const std::wstring& function,
            const std::wstring& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging )
{
	HRESULT hr = S_OK;

	std::wstringstream message;

	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrorMessages = nullptr;

	auto AsciiFunction = TextHelper::ToAscii(function);
	auto AsciiModel = TextHelper::ToAscii(model);

	// TODO: The compilation of shaders has to skip the warnings as errors 
	//       for the moment, since the new FXC.exe compiler in VS2012 is
	//       apparently more strict than before.

    u32 flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; // | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	// Get the current path to the shader folders, and add the filename to it.
	std::wstring filepath = FileSystem::getSingleton().GetShaderFolder() + filename;

	// Load the file into memory

	FileLoader SourceFile;
	if (SourceFile.Open(filepath)) 
	{
		message << "Unable to load shader from file: " << filepath;
		Log::Get().Write(message.str());
		return(nullptr);
	}

	if ( FAILED( hr = D3DCompile( 
		SourceFile.GetDataPtr(),
		SourceFile.GetDataSize(),
		nullptr,
		pDefines,
		nullptr,
		AsciiFunction.c_str(),
		AsciiModel.c_str(),
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