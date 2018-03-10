//***************************************************************************************
// FrameGraphTexture.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/FrameGraphResource.h"
#include "render/DataFormat.h"

namespace forward
{
	enum TextureBindPosition
	{
		TBP_Shader,
		TBP_RT,
		//TBP_RT_ONLY,
		TBP_DS,
		TBP_DS_ONLY
	};

	class FrameGraphTexture : public FrameGraphResource
	{
	public:
		FrameGraphTexture(const std::string& name, DataFormatType format, TextureBindPosition bind=TBP_Shader);

		DataFormatType	GetFormat() const;
		u32				GetMipLevelNum() const;
		bool			WantAutoGenerateMips() const;
		TextureBindPosition GetBindPosition() const;

	protected:
		DataFormatType	m_format = DF_UNKNOWN;
		u32				m_mipLevelNum = 1;
		bool			m_autoGenerateMip = false;
		TextureBindPosition m_bindPosition;
	};

	class FrameGraphTexture2D : public FrameGraphTexture
	{
	public:
		FrameGraphTexture2D(const std::string& name, DataFormatType format, u32 width, u32 height, TextureBindPosition);
		ResourceType GetResourceType() const override;

		u32 GetWidth() const;
		u32 GetHeight() const;

	protected:
		u32 m_width;
		u32 m_height;
	};
}