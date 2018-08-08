//***************************************************************************************
// FrameGraphShader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphShader.h"
#include "FileSystem.h"
#include "FileLoader.h"
#include "dx11/ShaderSystem/ShaderDX11.h"

using namespace forward;

FrameGraphShader::FrameGraphShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction)
	: m_entryFunction(entryFunction)
	, m_shaderFile(shaderFile)
{
	SetName(name);

	// Get the current path to the shader folders, and add the filename to it.
	std::wstring filepath = FileSystem::getSingleton().GetShaderFolder() + shaderFile;

	// Load the file into memory

	FileLoader SourceFile;
	if (SourceFile.Open(filepath))
	{
		assert(false);
	}

	m_shader = SourceFile.GetDataPtr();
}

void FrameGraphShader::PostSetDeviceObject()
{
	auto ptr = device_cast<ShaderDX11*>(this);
	m_CBufferLayouts.resize(ptr->GetCBuffers().size());
	auto i = 0U;
	for (auto const& cb : ptr->GetCBuffers())
	{
		m_Data[ConstantBufferShaderDataLookup].push_back(
			Data(FGOT_CONSTANT_BUFFER, cb.GetName(), cb.GetBindPoint(),
				cb.GetNumBytes(), 0, false));

		cb.GenerateLayout(m_CBufferLayouts[i]);
		++i;
	}

	//m_TBufferLayouts.resize(ptr->GetTBuffers().size());
	//i = 0U;
	//for (auto const& tb : ptr->GetTBuffers())
	//{
	//	mData[TextureBuffer::shaderDataLookup].push_back(
	//		Data(FGOT_TEXTURE_BUFFER, tb.GetName(), tb.GetBindPoint(),
	//			tb.GetNumBytes(), 0, false));

	//	tb.GenerateLayout(m_TBufferLayouts[i]);
	//	++i;
	//}

	for (auto const& tx : ptr->GetTextures())
	{
		m_Data[TextureSingleShaderDataLookup].push_back(
			Data(FGOT_TEXTURE, tx.GetName(), tx.GetBindPoint(), 0,
				tx.GetNumDimensions(), tx.IsGpuWritable()));
	}

	//for (auto const& ta : ptr->GetTextureArrays())
	//{
	//	mData[TextureArray::shaderDataLookup].push_back(
	//		Data(GT_TEXTURE_ARRAY, ta.GetName(), ta.GetBindPoint(), 0,
	//			ta.GetNumDimensions(), ta.IsGpuWritable()));
	//}
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
