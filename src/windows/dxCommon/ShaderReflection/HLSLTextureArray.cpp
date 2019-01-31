#include "HLSLTextureArray.h"

using namespace forward;

HLSLTextureArray::~HLSLTextureArray()
{
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