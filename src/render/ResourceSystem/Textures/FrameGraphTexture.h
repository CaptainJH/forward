//***************************************************************************************
// FrameGraphTexture.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/FrameGraphResource.h"
#include "render/DataFormat.h"

namespace forward
{
	class FrameGraphTexture : public FrameGraphResource
	{
	public:
		FrameGraphTexture(const std::string& name, DataFormatType format);

		DataFormatType GetFormat() const;

	protected:
		DataFormatType	m_format = DF_UNKNOWN;
	};

	class FrameGraphTexture2D : public FrameGraphTexture
	{
	public:
		FrameGraphTexture2D(const std::string& name, DataFormatType format, u32 width, u32 height);
		ResourceType GetResourceType() const override;

		u32 GetWidth() const;
		u32 GetHeight() const;

	protected:
		u32 m_width;
		u32 m_height;
	};
}