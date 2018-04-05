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
// ShaderFactoryDX11
//
//--------------------------------------------------------------------------------
#ifndef ShaderFactoryDX11_h
#define ShaderFactoryDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "ShaderDX.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class ShaderFactoryDX
	{
	public:
		~ShaderFactoryDX();

		static ID3DBlob* GenerateShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
            const std::wstring& model, const D3D_SHADER_MACRO* pDefines = nullptr, bool enablelogging = true );

	private:
		ShaderFactoryDX();
	};

};
//--------------------------------------------------------------------------------
#endif // ShaderFactoryDX11_h
//--------------------------------------------------------------------------------

