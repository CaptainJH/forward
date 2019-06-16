//***************************************************************************************
// FrameGraphTexture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphTexture.h"
#include "FileLoader.h"
#include "FileSystem.h"
#include "Log.h"

using namespace forward;

FrameGraphTexture::FrameGraphTexture(const std::string& name, DataFormatType format, u32 bind)
	: FrameGraphResource(name)
	, m_format(format)
	, m_bindPosition(bind)
	, m_fileFullPath(L"")
{
	m_type = FGOT_TEXTURE;
}

FrameGraphTexture::FrameGraphTexture(const std::string& name, const std::wstring& filename)
	: FrameGraphResource(name)
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

DataFormatType FrameGraphTexture::GetFormat() const
{
	return m_format;
}

u32 FrameGraphTexture::GetMipLevelNum() const
{
	return m_mipLevelNum;
}

u32 FrameGraphTexture::GetBindPosition() const
{
	return m_bindPosition;
}

bool FrameGraphTexture::WantAutoGenerateMips() const
{
	return m_autoGenerateMip;
}

u32 FrameGraphTexture::GetTotalElements(u32 numItems, u32 dim0, u32 dim1, u32 dim2) const
{
	auto numElementsPerItem = dim0 * dim1 * dim2;
	return numItems * numElementsPerItem;
}

bool FrameGraphTexture::IsFileTexture() const
{
	return m_fileFullPath != L"";
}

FrameGraphTexture2D::FrameGraphTexture2D(const std::string& name, DataFormatType format, 
	u32 width, u32 height, u32 bind, bool enableMSAA/*=false*/)
	: FrameGraphTexture(name, format, bind)
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

FrameGraphTexture2D::FrameGraphTexture2D(const std::string& name, const std::wstring& filename)
	: FrameGraphTexture(name, filename)
	, m_sampCount(1)
	, m_sampQuality(0)
{
	m_type = FGOT_TEXTURE2;
    
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
#endif
}

u32 FrameGraphTexture2D::GetWidth() const
{
	return m_width;
}

u32 FrameGraphTexture2D::GetHeight() const
{
	return m_height;
}

void FrameGraphTexture2D::EnableMSAA()
{
	m_sampCount = 8;
}

u32 FrameGraphTexture2D::GetSampCount() const
{
	return m_sampCount;
}


FrameGraphTextureCube::FrameGraphTextureCube(const std::string& name, DataFormatType format, u32 width, u32 height, u32 bind)
	: FrameGraphTexture(name, format, bind)
	, m_width(width)
	, m_height(height)
{
	m_type = FGOT_TEXTURECUBE;
	m_numElements = width * height * m_arraySize;
	m_elementSize = DataFormat::GetNumBytesPerStruct(format);
	Initialize(GetTotalElements(m_arraySize, width, height, 1), DataFormat::GetNumBytesPerStruct(format));
}

u32 FrameGraphTextureCube::GetWidth() const
{
	return m_width;
}

u32 FrameGraphTextureCube::GetHeight() const
{
	return m_height;
}

FrameGraphTextureCube::FrameGraphTextureCube(const std::string& name, const std::wstring& filename)
	: FrameGraphTexture(name, filename)
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
