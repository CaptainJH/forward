//***************************************************************************************
// ResourceConfig.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once


#include "Types.h"
#include "DeviceResource.h"
#include "DataFormat.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class ResourceConfig
	{
	public:

		virtual ResourceType GetResourceType() const = 0;
		virtual ~ResourceConfig() {}

		void SetFormat(DataFormatType state) { m_format = state; }
		DataFormatType GetFormat() const { return m_format; }

	protected:

		DataFormatType			m_format = DF_UNKNOWN;

	};


	class BufferConfig : public ResourceConfig
	{
	public:
		void SetBufferSize(u32 size) { m_size = size; }
		u32 GetBufferSize() const { return m_size; }

	protected:
		u32						m_size = 0;
	};

	class VertexBufferConfig : public BufferConfig
	{
	public:
		ResourceType GetResourceType() const override { return RT_VERTEXBUFFER; }
	};

	class IndexBufferConfig : public BufferConfig
	{
	public:
		ResourceType GetResourceType() const override { return RT_INDEXBUFFER; }
	};

	class ConstantBufferConfig : public BufferConfig
	{
	public:
		ResourceType GetResourceType() const override { return RT_CONSTANTBUFFER; }
	};

	class Texture1dConfig : public ResourceConfig
	{
	public:
		ResourceType GetResourceType() const override { return RT_TEXTURE1D; }
	};

	class Texture2dConfig : public ResourceConfig
	{
	public:
		Texture2dConfig() {}
		Texture2dConfig(const Texture2dConfig& config)
		{
			m_width = config.m_width;
			m_height = config.m_height;
			m_arraySize = config.m_arraySize;
			m_mipLevels = config.m_mipLevels;

			m_sampCount = config.m_sampCount;
			m_sampQuality = config.m_sampQuality;
		}

		ResourceType GetResourceType() const override { return RT_TEXTURE2D; }

		void SetWidth(u32 state) { m_width = state; }
		void SetHeight(u32 state) { m_height = state; }
		u32 GetWidth() const { return m_width; }
		u32 GetHeight() const { return m_height; }
		void SetMipLevels(u32 state) { m_mipLevels = state; }
		void SetArraySize(u32 state) { m_arraySize = state; }
		bool IsTextureArray() const { return m_arraySize > 1; }

		void SetSamp(u32 count, u32 quality) { m_sampCount = count; m_sampQuality = quality; }
		u32 GetSampCount() const { return m_sampCount; }
		u32 GetSampQuality() const { return m_sampQuality; }

	protected:

		u32		m_width = 0;
		u32		m_height = 0;
		u32		m_arraySize = 1;
		u32		m_mipLevels = 0;

		u32		m_sampCount = 1;
		u32		m_sampQuality = 0;
	};

	class TextureRTConfig : public Texture2dConfig
	{
	public:
		TextureRTConfig() {}
		TextureRTConfig(const TextureRTConfig& config)
			: Texture2dConfig(config)
		{}
	};

	class TextureDSConfig : public Texture2dConfig
	{
	public:
	};

	class Texture3dConfig : public ResourceConfig
	{
	public:
		Texture3dConfig() {}
		Texture3dConfig(const Texture3dConfig& config)
		{
			m_width = config.m_width;
			m_height = config.m_height;
			m_depth = config.m_depth;
			m_arraySize = config.m_arraySize;
			m_mipLevels = config.m_mipLevels;
		}

		ResourceType GetResourceType() const override { return RT_TEXTURE3D; }

		void SetWidth(u32 state) { m_width = state; }
		void SetHeight(u32 state) { m_height = state; }
		void SetDepth(u32 state) { m_depth = state; }
		u32 GetWidth() const { return m_width; }
		u32 GetHeight() const { return m_height; }
		u32 GetDepth() const { return m_depth; }
		void SetMipLevels(u32 state) { m_mipLevels = state; }
		void SetArraySize(u32 state) { m_arraySize = state; }
		bool IsTextureArray() const { return m_arraySize > 1; }

	protected:

		u32		m_width = 0;
		u32		m_height = 0;
		u32		m_depth = 0;
		u32		m_arraySize = 1;
		u32		m_mipLevels = 0;
	};


}