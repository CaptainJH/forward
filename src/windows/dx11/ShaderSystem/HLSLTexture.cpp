#include "HLSLTexture.h"

using namespace forward;

HLSLTexture::~HLSLTexture()
{
}

HLSLTexture::HLSLTexture(D3D11_SHADER_INPUT_BIND_DESC const& desc)
	:
	HLSLResource(desc, 0)
{
	Initialize(desc);
}

HLSLTexture::HLSLTexture(D3D11_SHADER_INPUT_BIND_DESC const& desc,
	u32 index)
	:
	HLSLResource(desc, index, 0)
{
	Initialize(desc);
}

void HLSLTexture::Initialize(D3D11_SHADER_INPUT_BIND_DESC const& desc)
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

u32 HLSLTexture::GetNumComponents() const
{
	return mNumComponents;
}

u32 HLSLTexture::GetNumDimensions() const
{
	return mNumDimensions;
}

bool HLSLTexture::IsGpuWritable() const
{
	return mGpuWritable;
}