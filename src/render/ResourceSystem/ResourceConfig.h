//***************************************************************************************
// ResourceConfig.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once


#include "Types.h"
#include "Resource.h"
#include "DataFormat.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class ResourceConfig
	{
	public:

		virtual ResourceType GetResourceType() const = 0;

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
		ResourceType GetResourceType() const override { return RT_TEXTURE2D; }

		void SetWidth(u32 state) { m_width = state; }
		void SetHeight(u32 state) { m_height = state; }
		u32 GetWidth() const { return m_width; }
		u32 GetHeight() const { return m_height; }
		void SetMipLevels(u32 state) { m_mipLevels = state; }
		void SetArraySize(u32 state) { m_arraySize = state; }
		bool IsTextureArray() const { return m_arraySize > 1; }

	protected:

		u32		m_width = 0;
		u32		m_height = 0;
		u32		m_arraySize = 1;
		u32		m_mipLevels = 0;
	};

	class TextureRTConfig : public Texture2dConfig
	{
	public:
	};

	class TextureDSConfig : public Texture2dConfig
	{
	public:
	};

	class Texture3dConfig : public ResourceConfig
	{
	public:
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