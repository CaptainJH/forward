//***************************************************************************************
// Texture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include <filesystem>
#include "Texture.h"
#include "FileLoader.h"
#include "FileSystem.h"
#include "Log.h"
#include "stb/stb_image.h"

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
	if (ext == L".png" || ext == L".jpg" || ext == L".bmp")
	{
		auto texPath = TextHelper::ToAscii(m_fileFullPath);
		i32 w, h, comp;
		const auto img = stbi_load(texPath.c_str(), &w, &h, &comp, 4); img;
		m_width = static_cast<u32>(w);
		m_height = static_cast<u32>(h);
		m_format = DF_R8G8B8A8_UNORM_SRGB;
		m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
		m_numElements = m_width * m_height;
		Initialize(m_numElements, m_elementSize);
		memcpy(m_data, img, m_numElements * DataFormat::GetNumBytesPerStruct(m_format));
		stbi_image_free((void*)img);
	}
#ifdef WINDOWS
	else if (ext == L".dds")
	{
		DDSFileLoader loader;
		if (loader.Open(m_fileFullPath))
		{
			return;
		}

		m_width = loader.GetImageWidth();
		m_height = loader.GetImageHeight();
		m_format = loader.GetImageFormat();
		m_mipLevelNum = loader.GetMipCount();

		bool isCubeMap = false;
		std::wstringstream wss;
		u32 dimension = 0;
		if (!loader.GetTextureDimension(dimension, isCubeMap))
		{
			assert(dimension == 2);
			assert(!isCubeMap);
			if (dimension != 2 || isCubeMap)
			{
				wss << L"Get Texture Dimension Failed! (" << filename << ")";
				auto text = wss.str();
				Log::Get().Write(text);
			}
		}
		else
		{
			wss << L"Get Texture Dimension Failed! (" << filename << ")";
			auto text = wss.str();
			Log::Get().Write(text);
		}

		assert(m_format != DataFormatType::DF_UNKNOWN);

		m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
		m_numElements = loader.GetImageContentSize() / m_elementSize;
		Initialize(m_numElements, m_elementSize);
		memcpy(m_data, loader.GetImageContentDataPtr(), loader.GetImageContentSize());
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
	DDSFileLoader loader;
	if (loader.Open(m_fileFullPath))
	{
		return;
	}

	m_width = loader.GetImageWidth();
	m_height = loader.GetImageHeight();
	m_format = loader.GetImageFormat();
	m_mipLevelNum = loader.GetMipCount();

	bool isCubeMap = false;
	std::wstringstream wss;
	u32 dimension = 0;
	if (!loader.GetTextureDimension(dimension, isCubeMap))
	{
		assert(dimension == 2);
		assert(isCubeMap);
		if (dimension != 2 || !isCubeMap)
		{
			wss << L"Get Texture Dimension Failed! (" << filename << ")";
			auto text = wss.str();
			Log::Get().Write(text);
		}
	}
	else
	{
		wss << L"Get Texture Dimension Failed! (" << filename << ")";
		auto text = wss.str();
		Log::Get().Write(text);
	}

	assert(m_format != DataFormatType::DF_UNKNOWN);

	m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
	m_numElements = loader.GetImageContentSize() / m_elementSize;
	Initialize(m_numElements, m_elementSize);
	memcpy(m_data, loader.GetImageContentDataPtr(), loader.GetImageContentSize());
#endif
}
