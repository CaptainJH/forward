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
// ShaderDX11
//
// The shader base class provides the mechanics of linking state information to
// a given shader.  Each type of shader utilizes various resources which are 
// managed by the renderer.  The specific resources are determined at shader load
// time via the reflection API and stored in the lists shown below.
//
// When a shader is bound to the pipeline, it's needed resources are updated if
// needed and then loaded into the corresponding shader stage.
//--------------------------------------------------------------------------------
#ifndef ShaderDX11_h
#define ShaderDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "dx11Util.h"
//--------------------------------------------------------------------------------
namespace forward
{
	enum ShaderType
	{
		VERTEX_SHADER = 0,
		HULL_SHADER = 1,
		DOMAIN_SHADER = 2,
		GEOMETRY_SHADER = 3,
		PIXEL_SHADER = 4,
		COMPUTE_SHADER = 5
	};


	enum ShaderMask
	{
		VERTEX_SHADER_MSK = 0x0001,
		HULL_SHADER_MSK = 0x0002,
		DOMAIN_SHADER_MSK = 0x0004,
		GEOMETRY_SHADER_MSK = 0x0008,
		PIXEL_SHADER_MSK = 0x0010,
		COMPUTE_SHADER_MSK = 0x0020
	};

	class ShaderDX11
	{
	public:
		ShaderDX11();
		virtual ~ShaderDX11();

		virtual ShaderType GetType() = 0;
		

		//void SetReflection( ShaderReflectionDX11* pReflection );
		//ShaderReflectionDX11* GetReflection( );

		std::wstring ToString();

	public:
		std::wstring							FileName;
		std::wstring							Function;
		std::wstring							ShaderModel;
		std::string								ShaderText;
		ID3DBlob*								m_pCompiledShader;
		//ShaderReflectionDX11*					m_pReflection;
	};
};
//--------------------------------------------------------------------------------
#endif // ShaderDX11_h
//--------------------------------------------------------------------------------

