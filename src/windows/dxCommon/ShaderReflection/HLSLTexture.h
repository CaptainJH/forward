#pragma once
#include "dxCommon/ShaderReflection/HLSLResource.h"

namespace forward
{
	class HLSLTexture : public HLSLResource
	{
	public:
		// Construction and destruction.
		virtual ~HLSLTexture();

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLTexture(D3D_SHADER_INPUT_BIND_DESC const& desc)
			: HLSLResource(desc, 0)
		{
			Initialize(desc);
		}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLTexture(D3D_SHADER_INPUT_BIND_DESC const& desc, u32 index)
			: HLSLResource(desc, index, 0)
		{
			Initialize(desc);
		}

		// Member access.
		u32 GetNumComponents() const;
		u32 GetNumDimensions() const;
		bool IsGpuWritable() const;

	private:
		template<class D3D_SHADER_INPUT_BIND_DESC>
		void Initialize(D3D_SHADER_INPUT_BIND_DESC const& desc)
		{
			mNumComponents = ((desc.uFlags >> 2) + 1);

			switch (desc.Dimension)
			{
			case D3D_SRV_DIMENSION_TEXTURE1D:
				mNumDimensions = 1;
				break;
			case D3D_SRV_DIMENSION_TEXTURE2D:
				mNumDimensions = 2;
				break;
			case D3D_SRV_DIMENSION_TEXTURE3D:
				mNumDimensions = 3;
				break;
			default:
				mNumDimensions = 0;
				break;
			}

			mGpuWritable = (desc.Type == D3D_SIT_UAV_RWTYPED);
		}

		u32 mNumComponents;
		u32 mNumDimensions;
		bool mGpuWritable;
	};
}