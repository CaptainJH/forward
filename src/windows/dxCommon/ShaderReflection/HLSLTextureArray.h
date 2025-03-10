#pragma once

#include "windows/dxCommon/ShaderReflection/HLSLResource.h"

namespace forward
{
	class HLSLTextureArray : public HLSLResource
	{
	public:
		// Construction and destruction.
		virtual ~HLSLTextureArray();

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLTextureArray(D3D_SHADER_INPUT_BIND_DESC const& desc)
			: HLSLResource(desc, 0)
		{
			Initialize(desc);
		}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLTextureArray(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 index)
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
			case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
				mNumDimensions = 1;
				break;
			case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
			case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
			case D3D_SRV_DIMENSION_TEXTURECUBE:
			case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
				mNumDimensions = 2;
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