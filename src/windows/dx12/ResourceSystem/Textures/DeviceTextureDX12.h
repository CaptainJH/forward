//***************************************************************************************
// DeviceTextureDX12.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx12/ResourceSystem/DeviceResourceDX12.h"

namespace forward
{
	class FrameGraphTexture;

	class DeviceTextureDX12 : public DeviceResourceDX12
	{
	public:
		DeviceTextureDX12(FrameGraphTexture* tex);

	protected:
		CD3DX12_CPU_DESCRIPTOR_HANDLE	m_hCpuDescriptorHandle;

	};
}
