//***************************************************************************************
// DeviceTextureDX12.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx12/ResourceSystem/DeviceResourceDX12.h"

namespace forward
{
	class Texture;

	class DeviceTextureDX12 : public DeviceResourceDX12
	{
	public:
		DeviceTextureDX12(Texture* tex);

		D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceViewHandle();

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHandle;
	};
}
