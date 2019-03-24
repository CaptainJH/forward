//***************************************************************************************
// DeviceTextureCubeDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DeviceTextureCubeDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

DeviceTextureCubeDX12::DeviceTextureCubeDX12(ID3D12Device* device, FrameGraphTextureCube* tex)
	: DeviceTextureDX12(tex)
{

}

shared_ptr<FrameGraphTextureCube> DeviceTextureCubeDX12::GetFrameGraphTextureCube()
{
	auto ptr = FrameGraphObject();
	auto p = dynamic_cast<FrameGraphTextureCube*>(ptr.get());

	return shared_ptr<FrameGraphTextureCube>(p);
}

void DeviceTextureCubeDX12::CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	m_srvHandle = RendererContext::GetCurrentRender()->AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tx.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = tx.MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	device->CreateShaderResourceView(m_deviceResPtr.Get(), &srvDesc, m_srvHandle);
}

void DeviceTextureCubeDX12::SyncCPUToGPU()
{

}