//***************************************************************************************
// DeviceTextureDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceTextureDX12.h"
#include "render/ResourceSystem/Texture.h"

using namespace forward;

DeviceTextureDX12::DeviceTextureDX12(Texture* tex)
	: DeviceResourceDX12(tex)
{

}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceTextureDX12::GetShaderResourceViewHandle()
{
	return m_srvHandle;
}