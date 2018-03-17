//***************************************************************************************
// FrameGraphShader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphShader.h"
#include "FileSystem.h"
#include "FileLoader.h"

using namespace forward;

FrameGraphShader::FrameGraphShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: m_entryFunction(entryFunction)
{
	SetName(name);

	// Get the current path to the shader folders, and add the filename to it.
	std::wstring filepath = FileSystem::getSingleton().GetShaderFolder() + shaderFile;

	// Load the file into memory

	FileLoader SourceFile;
	if (!SourceFile.Open(filepath))
	{
		assert(false);
	}

	m_shader = SourceFile.GetDataPtr();
}

FrameGraphVertexShader::FrameGraphVertexShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: FrameGraphShader(name, shaderFile, entryFunction)
{
	m_type = FGOT_VERTEX_SHADER;
}

FrameGraphPixelShader::FrameGraphPixelShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: FrameGraphShader(name, shaderFile, entryFunction)
{
	m_type = FGOT_PIXEL_SHADER;
}

const std::string& FrameGraphShader::GetShaderText() const
{
	return m_shader;
}

const std::wstring& FrameGraphShader::GetShaderFile() const
{
	return m_shaderFile;
}

const std::wstring& FrameGraphShader::GetShaderEntry() const
{
	return m_entryFunction;
}