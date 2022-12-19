//***************************************************************************************
// DeviceTextureDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceTextureDX12.h"
#include "RHI/ResourceSystem/Texture.h"

using namespace forward;

DeviceTextureDX12::DeviceTextureDX12(Texture* tex, DeviceDX12& d)
	: DeviceResourceDX12(tex, d)
{

}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceTextureDX12::GetShaderResourceViewHandle()
{
	return m_srvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceTextureDX12::GetUnorderedAccessViewHandle()
{
	return m_uavHandle;
}