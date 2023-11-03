//***************************************************************************************
// DeviceTexture2DDX12.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/Texture.h"
#include "DeviceTextureDX12.h"

namespace forward
{
	class Texture2D;
	class DeviceDX12;

	class DeviceTexture2DDX12 : public DeviceTextureDX12
	{
	public:
		static DeviceTexture2DDX12* BuildDeviceTexture2DDX12(DeviceDX12& d, const std::string& name, ID3D12Resource* tex, ResourceUsage usage = RU_IMMUTABLE);

		DeviceTexture2DDX12(Texture2D* tex, DeviceDX12& d);
		DeviceTexture2DDX12(ID3D12Resource* deviceTex, Texture2D* tex, DeviceDX12& d);

		void					SyncCPUToGPU() override;
		void					SyncGPUToCPU();

		shared_ptr<Texture2D> GetTexture2D();
		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetViewHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilViewHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE GetMipmapSRView(ID3D12Device* device, u32 mipLevel);
		D3D12_CPU_DESCRIPTOR_HANDLE GetMipmapUAView(ID3D12Device* device, u32 mipLevel);

	private:
		void CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
		void CreateUAView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
		void CreateDSView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
		void CreateDSSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
		void CreateRTView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);
		void CreateStaging(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx);

		D3D12_CPU_DESCRIPTOR_HANDLE		m_rtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE		m_dsvHandle;
	};
}