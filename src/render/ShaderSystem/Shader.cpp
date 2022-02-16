//***************************************************************************************
// Shader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "Shader.h"
#include "FileSystem.h"
#include "FileLoader.h"

using namespace forward;

Shader::Shader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	:
#ifdef WINDOWS
	  m_shaderFile(shaderFile + L".hlsl")
#elif MACOS
	  m_shaderFile(shaderFile + L".metal")
#endif
    , m_entryFunction(entryFunction)
{
	SetName(name);

	// Get the current path to the shader folders, and add the filename to it.
	std::wstring filepath = FileSystem::getSingleton().GetShaderFolder() + m_shaderFile;

	// Load the file into memory

	FileLoader SourceFile;
	if (SourceFile.Open(filepath))
	{
		assert(false);
	}

	m_shader = SourceFile.GetDataPtr();
}

Shader::Data::Data(GraphicsObjectType inType, std::string const& inName,
	i32 inBindPoint, i32 inNumBytes, u32 inExtra,
	bool inIsGpuWritable)
	:
	type(inType),
	name(inName),
	bindPoint(inBindPoint),
	numBytes(inNumBytes),
	extra(inExtra),
	isGpuWritable(inIsGpuWritable)
{
}

VertexShader::VertexShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_VERTEX_SHADER;
}

PixelShader::PixelShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_PIXEL_SHADER;
}

GeometryShader::GeometryShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_GEOMETRY_SHADER;
}

ComputeShader::ComputeShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_COMPUTE_SHADER;
}

const std::string& Shader::GetShaderText() const
{
	return m_shader;
}

const std::wstring& Shader::GetShaderFile() const
{
	return m_shaderFile;
}

const std::wstring& Shader::GetShaderEntry() const
{
	return m_entryFunction;
}
