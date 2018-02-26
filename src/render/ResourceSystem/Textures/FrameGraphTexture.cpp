//***************************************************************************************
// FrameGraphTexture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphTexture.h"

using namespace forward;

FrameGraphTexture::FrameGraphTexture(const std::string& name, DataFormatType format)
	: FrameGraphResource(name)
	, m_format(format)
{
	m_type = FGOT_TEXTURE;
}

DataFormatType FrameGraphTexture::GetFormat() const
{
	return m_format;
}

FrameGraphTexture2D::FrameGraphTexture2D(const std::string& name, DataFormatType format, u32 width, u32 height)
	: FrameGraphTexture(name, format)
	, m_width(width)
	, m_height(height)
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