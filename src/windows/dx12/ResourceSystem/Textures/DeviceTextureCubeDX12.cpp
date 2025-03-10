//***************************************************************************************
// DeviceTextureCubeDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DeviceTextureCubeDX12.h"
#include "windows/dx12/DeviceDX12.h"

using namespace forward;

DeviceTextureCubeDX12::DeviceTextureCubeDX12(TextureCube* tex, DeviceDX12& d)
	: DeviceTextureDX12(tex, d)
{
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = tex->GetWidth();
	desc.Height = tex->GetHeight();
	desc.DepthOrArraySize = 6;
	desc.MipLevels = static_cast<u16>(tex->GetMipLevelNum());
	desc.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	const auto TBP = tex->GetBindPosition();
	assert(TBP & TBP_Shader);

	auto device = m_device.GetDevice();

	CD3DX12_HEAP_PROPERTIES properties(D3D12_HEAP_TYPE_DEFAULT);
	HR(device->CreateCommittedResource(
		&properties,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&desc,
		GetResourceState(),
		nullptr,
		IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())
	));
	m_gpuVirtualAddress = m_deviceResPtr->GetGPUVirtualAddress();

	if (tex->GetUsage() == ResourceUsage::RU_IMMUTABLE && tex->GetData())
	{
		const auto num2DSubresource = 6 * tex->GetMipLevelNum();
		u64 uploadSize = 0U;
		device->GetCopyableFootprints(&desc, 0, num2DSubresource, 0, nullptr, nullptr, nullptr, &uploadSize);
		auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

		// In order to copy CPU memory data into our default buffer, we need to create
		// an intermediate upload heap. 
		CD3DX12_HEAP_PROPERTIES heap_staging_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		HR(device->CreateCommittedResource(
			&heap_staging_properties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_stagingResPtr.GetAddressOf())));

		SyncCPUToGPU();
	}

	if(TBP & TBP_Shader)
	{
		CreateSRView(device, desc);
	}
}

shared_ptr<TextureCube> DeviceTextureCubeDX12::GetFrameGraphTextureCube()
{
	auto ptr = GraphicsObject();
	auto p = dynamic_cast<TextureCube*>(ptr.get());

	return shared_ptr<TextureCube>(p);
}

void DeviceTextureCubeDX12::CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	m_srvHandle = m_device.AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
	auto resTex = GetFrameGraphTextureCube();
	auto cmdList = m_device.DeviceCommandList();
	const auto num2DSubresource = 6 * resTex->GetMipLevelNum();

	// Describe the data we want to copy into the default buffer.
	std::unique_ptr<D3D12_SUBRESOURCE_DATA[]> initData(new D3D12_SUBRESOURCE_DATA[num2DSubresource]);
	assert(initData);

	u32 skipMip = 0;
	u32 twidth = 0;
	u32 theight = 0;
	u32 tdepth = 0;
	FillInitDataDX12(resTex->GetWidth(), resTex->GetHeight(), 1, resTex->GetMipLevelNum(), 6, resTex->GetFormat(), 0, resTex->GetNumBytes(),
		resTex->GetData(), twidth, theight, tdepth, skipMip, initData.get());

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	auto transitionToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
		GetResourceState(),
		D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &transitionToCopyDest);
	UpdateSubresources(cmdList, m_deviceResPtr.Get(), m_stagingResPtr.Get(), 0, 0, num2DSubresource, initData.get());
	auto transitionBack = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &transitionBack);
	SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
}