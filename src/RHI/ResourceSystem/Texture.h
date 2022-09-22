//***************************************************************************************
// Texture.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/Resource.h"
#include "RHI/DataFormat.h"

namespace forward
{
	enum TextureBindPosition
	{
		TBP_None	= 0L,
		TBP_Shader	= 0x8L,
		TBP_RT		= 0x20L,
		TBP_DS		= 0x40L,
	};

	class Texture : public Resource
	{
	public:
		Texture(const std::string& name, DataFormatType format, u32 bind=TBP_Shader);
		Texture(const std::string& name, const std::wstring& filename);

		DataFormatType	GetFormat() const;
		u32				GetMipLevelNum() const;
		bool			WantAutoGenerateMips() const;
		u32				GetBindPosition() const;
		bool			IsFileTexture() const;

	protected:
		DataFormatType	m_format = DF_UNKNOWN;
		u32				m_mipLevelNum = 1;
		bool			m_autoGenerateMip = false;
		u32				m_bindPosition;

		std::wstring	m_fileFullPath;

		u32				GetTotalElements(u32 numItems, u32 dim0, u32 dim1, u32 dim2) const;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(const std::string& name, DataFormatType format, u32 width, u32 height, u32 bp, bool enableMSAA=false);
		Texture2D(const std::string& name, const std::wstring& filename);

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

	class TextureCube : public Texture
	{
	public:
		TextureCube(const std::string& name, DataFormatType format, u32 width, u32 height, u32 bind);
		TextureCube(const std::string& name, const std::wstring& filename);

		u32 GetWidth() const;
		u32 GetHeight() const;

	protected:
		u32 m_width;
		u32 m_height;

		static const u32 m_arraySize = 6;
	};
}