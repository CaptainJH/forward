#pragma once
#include "dx11/dx11Util.h"
#include <d3d11shader.h>

namespace forward
{
	class HLSLResource
	{
	public:
		// Abstract base class, destructor.
		virtual ~HLSLResource();
	protected:
		// Construction.
		HLSLResource(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes);
		HLSLResource(D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 index,
			u32 numBytes);

	public:
		struct Description
		{
			std::string name;
			D3D_SHADER_INPUT_TYPE type;
			u32 bindPoint;
			u32 bindCount;
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