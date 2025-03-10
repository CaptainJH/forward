//***************************************************************************************
// DeviceTextureCubeDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/Texture.h"
#include "DeviceTextureDX12.h"
#include "windows/dx12/dx12Util.h"

namespace forward
{
	class TextureCube;
	class DeviceDX12;

	class DeviceTextureCubeDX12 : public DeviceTextureDX12
	{
	public:
		DeviceTextureCubeDX12(TextureCube* tex, DeviceDX12& d);

		shared_ptr<TextureCube> GetFrameGraphTextureCube();

		void SyncCPUToGPU() override;

	private:
		void CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
	};
}