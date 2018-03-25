#pragma once
#include "HLSLResource.h"

namespace forward
{
	class HLSLTexture : public HLSLResource
	{
	public:
		// Construction and destruction.
		virtual ~HLSLTexture();

		HLSLTexture(D3D11_SHADER_INPUT_BIND_DESC const& desc);

		HLSLTexture(D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 index);

		// Member access.
		u32 GetNumComponents() const;
		u32 GetNumDimensions() const;
		bool IsGpuWritable() const;

	private:
		void Initialize(D3D11_SHADER_INPUT_BIND_DESC const& desc);

		u32 mNumComponents;
		u32 mNumDimensions;
		bool mGpuWritable;
	};
}