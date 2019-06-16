//***************************************************************************************
// FrameGraphShader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphShader.h"
#include "FileSystem.h"
#include "FileLoader.h"

using namespace forward;

FrameGraphShader::FrameGraphShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: m_entryFunction(entryFunction)
#ifdef WINDOWS
	, m_shaderFile(shaderFile + L".hlsl")
#elif MACOS
	, m_shaderFile(shaderFile + L".metal")
#endif
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

FrameGraphShader::Data::Data(FrameGraphObjectType inType, std::string const& inName,
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

FrameGraphGeometryShader::FrameGraphGeometryShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: FrameGraphShader(name, shaderFile, entryFunction)
{
	m_type = FGOT_GEOMETRY_SHADER;
}

FrameGraphComputeShader::FrameGraphComputeShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: FrameGraphShader(name, shaderFile, entryFunction)
{
	m_type = FGOT_COMPUTE_SHADER;
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
