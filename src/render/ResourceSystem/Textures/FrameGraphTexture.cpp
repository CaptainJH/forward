//***************************************************************************************
// FrameGraphTexture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphTexture.h"
#include "FileLoader.h"

using namespace forward;

FrameGraphTexture::FrameGraphTexture(const std::string& name, DataFormatType format, u32 bind)
	: FrameGraphResource(name)
	, m_format(format)
	, m_bindPosition(bind)
{
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

FrameGraphTexture2D::FrameGraphTexture2D(const std::string& name)
	: FrameGraphTexture(name, DF_UNKNOWN, TBP_Shader)
	, m_sampCount(1)
	, m_sampQuality(0)
{
	m_type = FGOT_TEXTURE2;
}

void FrameGraphTexture2D::LoadFromDDS(const std::wstring& filename)
{
	DDSFileLoader loader;	
	if (loader.Open(filename))
	{
		return;
	}

	m_width = loader.GetImageWidth();
	m_height = loader.GetImageHeight();
	m_format = loader.GetImageFormat();
	assert(m_format != DataFormatType::DF_UNKNOWN);

	m_elementSize = DataFormat::GetNumBytesPerStruct(m_format);
	m_numElements = loader.GetImageContentSize() / m_elementSize;
	Initialize(m_numElements, m_elementSize);
	memcpy(m_data, loader.GetImageContentDataPtr(), loader.GetImageContentSize());
}

ResourceType FrameGraphTexture2D::GetResourceType() const
{
	return RT_TEXTURE2D;
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