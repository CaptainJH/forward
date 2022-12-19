//***************************************************************************************
// DeviceTextureDX12.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx12/ResourceSystem/DeviceResourceDX12.h"

namespace forward
{
	class Texture;
	class DeviceDX12;

	class DeviceTextureDX12 : public DeviceResourceDX12
	{
	public:
		DeviceTextureDX12(Texture* tex, DeviceDX12& d);

		D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceViewHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessViewHandle();

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE		m_uavHandle;
	};
}
