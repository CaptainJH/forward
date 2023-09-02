//***************************************************************************************
// Texture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#define TINYDDSLOADER_IMPLEMENTATION
#include <filesystem>
#include "Texture.h"
#include "FileLoader.h"
#include "FileSystem.h"
#include "Log.h"
#include "stb/stb_image.h"

#ifdef WINDOWS
#include <DirectXTex.h>
#endif

using namespace forward;

Texture::Texture(const std::string& name, DataFormatType format, u32 bind)
	: Resource(name)
	, m_format(format)
	, m_bindPosition(bind)
	, m_fileFullPath(L"")
{
	m_type = FGOT_TEXTURE;
}

Texture::Texture(const std::string& name, const std::wstring& filename)
	: Resource(name)
	, m_bindPosition(TBP_Shader)
{
	if (std::filesystem::exists(std::filesystem::path(filename)))
	{
		m_fileFullPath = filename;
	}
	else if (std::filesystem::path(filename).has_parent_path())
	{
		auto fileFullPath_data = forward::FileSystem::getSingleton().GetModelsFolder() + filename;
		auto fileFullPath_deps = forward::FileSystem::getSingleton().GetExternFolder() + filename;
		auto fileFullPath_tex = forward::FileSystem::getSingleton().GetTextureFolder() + filename;
		if (forward::FileSystem::getSingleton().FileExists(fileFullPath_data))
			m_fileFullPath = fileFullPath_data;
		else if (forward::FileSystem::getSingleton().FileExists(fileFullPath_deps))
			m_fileFullPath = fileFullPath_deps;
		else if (forward::FileSystem::getSingleton().FileExists(fileFullPath_tex))
			m_fileFullPath = fileFullPath_tex;
	}
	else
		m_fileFullPath = forward::FileSystem::getSingletonPtr()->GetTextureFolder() + filename;
	if (!forward::FileSystem::getSingletonPtr()->FileExists(m_fileFullPath))
	{
		m_fileFullPath = L"";

		std::wstringstream wss;
		wss << L"DDS file: " << filename << L" doesn't exist.";
		auto text = wss.str();
		Log::Get().Write(text);
	}

	m_type = FGOT_TEXTURE;
}

DataFormatType Texture::GetFormat() const
{
	return m_format;
}

u32 Texture::GetMipLevelNum() const
{
	return m_mipLevelNum;
}

u32 Texture::GetBindPosition() const
{
	return m_bindPosition;
}

bool Texture::WantAutoGenerateMips() const
{
	return m_autoGenerateMip;
}

u32 Texture::GetTotalElements(u32 numItems, u32 dim0, u32 dim1, u32 dim2) const
{
	auto numElementsPerItem = dim0 * dim1 * dim2;
	return numItems * numElementsPerItem;
}

bool Texture::IsFileTexture() const
{
	return m_fileFullPath != L"";
}

Texture2D::Texture2D(const std::string& name, DataFormatType format, 
	u32 width, u32 height, u32 bind, bool enableMSAA/*=false*/)
	: Texture(name, format, bind)
	, m_width(width)
	, m_height(height)
	, m_sampCount(enableMSAA ? 8: 1)
	, m_sampQuality(0)
{
	m_type = FGOT_TEXTURE2;
	m_numElements = width * height;
	m_elementSize = DataFormat::GetNumBytesPerStruct(format);
	Initialize(GetTotalElements(1, width, height, 1), DataFormat::GetNumBytesPerStruct(format));
}

Texture2D::Texture2D(const std::string& name, const std::wstring& filename)
	: Texture(name, filename)
	, m_sampCount(1)
	, m_sampQuality(0)
{
	m_type = FGOT_TEXTURE2;

	const auto ext = std::filesystem::path(m_fileFullPath).extension();
	if (ext == L".png" || ext == L".jpg" || ext == L".bmp" || ext == L".tga" || ext == L".hdr")
	{
		auto texPath = TextHelper::ToAscii(m_fileFullPath);
		i32 w, h, comp;
		const auto img = stbi_load(texPath.c_str(), &w, &h, &comp, 4);
		m_width = static_cast<u32>(w);
		m_height = static_cast<u32>(h);
		m_format = DF_R8G8B8A8_UNORM;
		m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
		m_numElements = m_width * m_height;
		Initialize(m_numElements, m_elementSize);
		memcpy(m_data, img, m_numElements * DataFormat::GetNumBytesPerStruct(m_format));
		stbi_image_free((void*)img);
	}
#ifdef WINDOWS
	else if (ext == L".dds" || ext == L".DDS")
	{
		DirectX::TexMetadata  metadata;
		DirectX::ScratchImage scratchImage;
#ifdef _DEBUG
		auto ret =
#endif
			DirectX::LoadFromDDSFile(m_fileFullPath.c_str(), DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage);
#ifdef _DEBUG
		assert(SUCCEEDED(ret));
#endif

		m_width = (u32)metadata.width;
		m_height = (u32)metadata.height;
		m_format = (forward::DataFormatType)metadata.format;
		m_mipLevelNum = (u32)metadata.mipLevels;
		assert(!metadata.IsCubemap());
		if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		{
#ifdef _DEBUG
			for (auto i = 1; i < scratchImage.GetImageCount(); ++i)
			{
				const auto& img = scratchImage.GetImages()[i];
				const auto& img_prev = scratchImage.GetImages()[i - 1];
				assert(img.pixels == img_prev.pixels + img_prev.slicePitch);
			}
#endif

			m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
			m_numElements = (u32)scratchImage.GetPixelsSize() / m_elementSize;
			Initialize(m_numElements, m_elementSize);
			memcpy(m_data, scratchImage.GetPixels(), scratchImage.GetPixelsSize());
		}
	}
#endif
}

u32 Texture2D::GetWidth() const
{
	return m_width;
}

u32 Texture2D::GetHeight() const
{
	return m_height;
}

void Texture2D::EnableMSAA()
{
	m_sampCount = 8;
}

u32 Texture2D::GetSampCount() const
{
	return m_sampCount;
}


TextureCube::TextureCube(const std::string& name, DataFormatType format, u32 width, u32 height, u32 bind)
	: Texture(name, format, bind)
	, m_width(width)
	, m_height(height)
{
	m_type = FGOT_TEXTURECUBE;
	m_numElements = width * height * m_arraySize;
	m_elementSize = DataFormat::GetNumBytesPerStruct(format);
	Initialize(GetTotalElements(m_arraySize, width, height, 1), DataFormat::GetNumBytesPerStruct(format));
}

u32 TextureCube::GetWidth() const
{
	return m_width;
}

u32 TextureCube::GetHeight() const
{
	return m_height;
}

TextureCube::TextureCube(const std::string& name, const std::wstring& filename)
	: Texture(name, filename)
{
	m_type = FGOT_TEXTURECUBE;

#ifdef WINDOWS
	DirectX::TexMetadata  metadata;
	DirectX::ScratchImage scratchImage;
#if _DEBUG
	auto ret = 
#endif
		DirectX::LoadFromDDSFile(m_fileFullPath.c_str(), DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage);
#if _DEBUG
	assert(SUCCEEDED(ret));
#endif

	m_width = (u32)metadata.width;
	m_height = (u32)metadata.height;
	m_format = (forward::DataFormatType)metadata.format;
	m_mipLevelNum = (u32)metadata.mipLevels;
	assert(metadata.IsCubemap());
#ifdef _DEBUG
	for (auto i = 1; i < scratchImage.GetImageCount(); ++i)
	{
		const auto& img = scratchImage.GetImages()[i];
		const auto& img_prev = scratchImage.GetImages()[i - 1];
		assert(img.pixels == img_prev.pixels + img_prev.slicePitch);
	}
#endif

	m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
	m_numElements = (u32)scratchImage.GetPixelsSize() / m_elementSize;
	Initialize(m_numElements, m_elementSize);
	memcpy(m_data, scratchImage.GetPixels(), scratchImage.GetPixelsSize());
#endif
}
