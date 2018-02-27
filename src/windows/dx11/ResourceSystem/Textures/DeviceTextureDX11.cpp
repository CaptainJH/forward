//***************************************************************************************
// DeviceTextureDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceTextureDX11.h"

using namespace forward;

ShaderResourceViewComPtr DeviceTextureDX11::GetSRView() const
{
	return m_srv;
}

UnorderedAccessViewComPtr DeviceTextureDX11::GetUAView() const
{
	return m_uav;
}

void DeviceTextureDX11::CopyCPUToGPU(u8* /*srcData*/, u32 /*srcDataSize*/)
{
	/// TODO:
}

void DeviceTextureDX11::CopyGPUToCPU(u8* /*dstData*/, u32 /*dstDataSize*/)
{
	/// TODO:
}