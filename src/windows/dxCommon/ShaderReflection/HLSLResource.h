#pragma once
#include <d3dcommon.h>
#include "utilities/Utils.h"

namespace forward
{
	class HLSLResource
	{
	public:
		// Abstract base class, destructor.
		virtual ~HLSLResource();
	protected:
		// Construction.
		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLResource(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes)
			: m_numBytes(numBytes)
		{
			m_desc.name = std::string(desc.Name);
			m_desc.bindPoint = desc.BindPoint;
			m_desc.bindCount = desc.BindCount;
			m_desc.type = desc.Type;
			m_desc.flags = desc.uFlags;
			m_desc.returnType = desc.ReturnType;
			m_desc.dimension = desc.Dimension;
			m_desc.numSamples = desc.NumSamples;
			m_desc.space = desc.Space;
		}
		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLResource(D3D_SHADER_INPUT_BIND_DESC const& desc, u32 index,
			u32 numBytes)
			: m_numBytes(numBytes)
		{
			m_desc.name = std::string(desc.Name) + "[" + std::to_string(index) + "]";
			m_desc.bindPoint = desc.BindPoint + index;
			m_desc.bindCount = 1;
			m_desc.type = desc.Type;
			m_desc.flags = desc.uFlags;
			m_desc.returnType = desc.ReturnType;
			m_desc.dimension = desc.Dimension;
			m_desc.numSamples = desc.NumSamples;
			m_desc.space = desc.Space;
		}

	public:
		struct Description
		{
			std::string name;
			D3D_SHADER_INPUT_TYPE type;
			u32 bindPoint;
			u32 bindCount;
			u32 space;
			u32 flags;
			D3D_RESOURCE_RETURN_TYPE returnType;
			D3D_SRV_DIMENSION dimension;
			u32 numSamples;
		};

		// Member access.
		std::string const& GetName() const;
		D3D_SHADER_INPUT_TYPE GetType() const;
		u32 GetBindPoint() const;
		u32 GetBindCount() const;
		u32 GetSpace() const;
		u32 GetFlags() const;
		D3D_RESOURCE_RETURN_TYPE GetReturnType() const;
		D3D_SRV_DIMENSION GetDimension() const;
		u32 GetNumSamples() const;
		u32 GetNumBytes() const;

		// Print to a text file for human readability.
		virtual void Print(std::ofstream& output) const;

	private:
		Description m_desc;
		u32 m_numBytes;

		// Support for Print.
		static std::string const msSIType[];
		static std::string const msReturnType[];
		static std::string const msSRVDimension[];
	};
}