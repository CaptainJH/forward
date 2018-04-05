#include "HLSLTextureArray.h"

using namespace forward;

HLSLTextureArray::~HLSLTextureArray()
{
}

HLSLTextureArray::HLSLTextureArray(D3D11_SHADER_INPUT_BIND_DESC const& desc)
	:
	HLSLResource(desc, 0)
{
	Initialize(desc);
}

HLSLTextureArray::HLSLTextureArray(D3D11_SHADER_INPUT_BIND_DESC const& desc,
	u32 index)
	:
	HLSLResource(desc, index, 0)
{
	Initialize(desc);
}

void HLSLTextureArray::Initialize(D3D11_SHADER_INPUT_BIND_DESC const& desc)
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

u32 HLSLTextureArray::GetNumComponents() const
{
	return mNumComponents;
}

u32 HLSLTextureArray::GetNumDimensions() const
{
	return mNumDimensions;
}

bool HLSLTextureArray::IsGpuWritable() const
{
	return mGpuWritable;
}