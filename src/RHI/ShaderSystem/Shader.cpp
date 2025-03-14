//***************************************************************************************
// Shader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "Shader.h"
#include "utilities/FileSystem.h"
#include "utilities/FileLoader.h"

using namespace forward;

Shader::Shader(const i8* name, const WString shaderFile, const i8* entryFunction)
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

Shader::Shader(const i8* name, const String shaderText)
	: m_entryFunction("main")
	, m_shaderFile(L"")
	, m_shader(shaderText)
{
	SetName(name);
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

VertexShader::VertexShader(const i8* name, const WString shaderFile, const i8* entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_VERTEX_SHADER;
}

VertexShader::VertexShader(const i8* name, const String shaderText)
	: Shader(name, shaderText)
{
	m_type = FGOT_VERTEX_SHADER;
#ifdef WINDOWS
	WString file = TextHelper::ToUnicode(String(name));
	m_shaderFile = file + L".hlsl";
#endif
}

PixelShader::PixelShader(const i8* name, const WString shaderFile, const i8* entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_PIXEL_SHADER;
}

PixelShader::PixelShader(const i8* name, const String shaderText)
	: Shader(name, shaderText)
{
	m_type = FGOT_PIXEL_SHADER;
#ifdef WINDOWS
	WString file = TextHelper::ToUnicode(String(name));
	m_shaderFile = file + L".hlsl";
#endif
}

GeometryShader::GeometryShader(const i8* name, const WString shaderFile, const i8* entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_GEOMETRY_SHADER;
}

ComputeShader::ComputeShader(const i8* name, const WString shaderFile, const i8* entryFunction)
	: Shader(name, shaderFile, entryFunction)
{
	m_type = FGOT_COMPUTE_SHADER;
}

RaytracingShaders::RaytracingShaders(const i8* name, const WString shaderFile)
	: Shader(name, shaderFile, "")
{
	m_type = FGOT_RT_SHADER;
}

const std::string& Shader::GetShaderText() const
{
	return m_shader;
}

const std::wstring& Shader::GetShaderFile() const
{
	return m_shaderFile;
}

const std::string& Shader::GetShaderEntry() const
{
	return m_entryFunction;
}
