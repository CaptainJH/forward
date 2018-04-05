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
		TBP_None	= 0L,
		TBP_Shader	= 0x8L,
		TBP_RT		= 0x20L,
		TBP_DS		= 0x40L,
	};

	class FrameGraphTexture : public FrameGraphResource
	{
	public:
		FrameGraphTexture(const std::string& name, DataFormatType format, u32 bind=TBP_Shader);

		DataFormatType	GetFormat() const;
		u32				GetMipLevelNum() const;
		bool			WantAutoGenerateMips() const;
		u32				GetBindPosition() const;

	protected:
		DataFormatType	m_format = DF_UNKNOWN;
		u32				m_mipLevelNum = 1;
		bool			m_autoGenerateMip = false;
		u32				m_bindPosition;
	};

	class FrameGraphTexture2D : public FrameGraphTexture
	{
	public:
		FrameGraphTexture2D(const std::string& name, DataFormatType format, u32 width, u32 height, u32 bp, bool enableMSAA=false);
		ResourceType GetResourceType() const override;

		u32 GetWidth() const;
		u32 GetHeight() const;
		void EnableMSAA();
		u32 GetSampCount() const;

	protected:
		u32 m_width;
		u32 m_height;

		u32 m_sampCount;
		u32 m_sampQuality;
	};
}