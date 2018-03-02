//***************************************************************************************
// ShaderDX.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "dxCommon/d3dUtil.h"
#include <d3dCompiler.h>
#include "render/ResourceSystem/DeviceObject.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class FrameGraphObject;

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

	class ShaderDX : public DeviceObject
	{
	public:
		/// TODO: just for backward compatibility to project forwardDX11_Hieroglyph
		ShaderDX();
		ShaderDX(forward::FrameGraphObject* obj);
		virtual ~ShaderDX();

		virtual ShaderType GetType() = 0;

		ID3DBlob*	GetCompiledCode();	

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

