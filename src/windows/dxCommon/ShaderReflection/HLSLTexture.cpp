#include "HLSLTexture.h"

using namespace forward;

HLSLTexture::~HLSLTexture()
{
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