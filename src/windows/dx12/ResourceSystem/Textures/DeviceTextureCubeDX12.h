//***************************************************************************************
// DeviceTextureCubeDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/Texture.h"
#include "DeviceTextureDX12.h"
#include "dx12/dx12Util.h"

namespace forward
{
	class TextureCube;

	class DeviceTextureCubeDX12 : public DeviceTextureDX12
	{
	public:
		DeviceTextureCubeDX12(ID3D12Device* device, TextureCube* tex);

		shared_ptr<TextureCube> GetFrameGraphTextureCube();

		void SyncCPUToGPU() override;

	private:
		void CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
	};
}