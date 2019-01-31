#pragma once
#include "HLSLResource.h"


namespace forward
{
	class HLSLShaderVariable
	{
	public:
		struct Description
		{
			std::string name;
			u32 offset;
			u32 numBytes;
			u32 flags;
			u32 textureStart;
			u32 textureNumSlots;
			u32 samplerStart;
			u32 samplerNumSlots;
			std::vector<u8> defaultValue;
		};

		// Construction.  Shader variables are reported for constant buffers,
		// texture buffers, and structs defined in the shaders (resource
		// binding information).
		HLSLShaderVariable();

		// Deferred construction for shader reflection.  This function is
		// intended to be write-once.
		template<class D3D_SHADER_VARIABLE_DESC>
		void SetDescription(D3D_SHADER_VARIABLE_DESC const& desc)
		{
			m_desc.name = std::string(desc.Name ? desc.Name : "");
			m_desc.offset = desc.StartOffset;
			m_desc.numBytes = desc.Size;
			m_desc.flags = desc.uFlags;
			m_desc.textureStart = desc.StartTexture;
			m_desc.textureNumSlots = desc.TextureSize;
			m_desc.samplerStart = desc.StartSampler;
			m_desc.samplerNumSlots = desc.SamplerSize;
			if (desc.DefaultValue && desc.Size > 0)
			{
				m_desc.defaultValue.resize(desc.Size);
				memcpy(&m_desc.defaultValue[0], desc.DefaultValue, desc.Size);
			}
		}

		// Member access.
		std::string const& GetName() const;
		u32 GetOffset() const;
		u32 GetNumBytes() const;
		u32 GetFlags() const;
		u32 GetTextureStart() const;
		u32 GetTextureNumSlots() const;
		u32 GetSamplerStart() const;
		u32 GetSamplerNumSlots() const;
		std::vector<u8> const& GetDefaultValue() const;

		// Print to a text file for human readability.
		void Print(std::ofstream& output) const;

	private:
		Description m_desc;

	};
}