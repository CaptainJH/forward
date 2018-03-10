//***************************************************************************************
// FrameGraphShader.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/FrameGraph/FrameGraphObject.h"

namespace forward
{

	class FrameGraphShader : public FrameGraphObject
	{
	public:
		FrameGraphShader(const std::string& name, const std::wstring& shaderFile);

		const std::string& GetShaderText() const;
		const std::wstring& GetShaderFile() const;
		const std::wstring& GetShaderEntry() const;

	protected:
		std::wstring m_shaderFile;
		std::wstring m_entryFunction;
		std::string m_shader;
		
	};


	class FrameGraphVertexShader : public FrameGraphShader
	{
	public:
		FrameGraphVertexShader(const std::string& name, const std::wstring& shaderFile);

	};

	class FrameGraphPixelShader : public FrameGraphShader
	{
	public:
		FrameGraphPixelShader(const std::string& name, const std::wstring& shaderFile);
	};

}