//***************************************************************************************
// FrameGraphTexture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphTexture.h"

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

FrameGraphTexture2D::FrameGraphTexture2D(const std::string& name, DataFormatType format, 
	u32 width, u32 height, u32 bind, bool enableMSAA/*=false*/)
	: FrameGraphTexture(name, format, bind)
	, m_width(width)
	, m_height(height)
	, m_sampCount(enableMSAA ? 8: 1)
	, m_sampQuality(0)
{
	m_type = FGOT_TEXTURE2;
	//Initialize(width * height, DataFormat::GetNumBytesPerStruct(format));
	m_numElements = width * height;
	m_elementSize = DataFormat::GetNumBytesPerStruct(format);
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