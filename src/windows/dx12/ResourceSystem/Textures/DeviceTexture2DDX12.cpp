//***************************************************************************************
// DeviceTexture2DDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

// shouldn't generate any shared_ptr of the FrameGraphObject within this function call
// because this is a very special function (create device object first, then the frame graph object)
// normally the reference count of the frame graph object is zero within this function, 
// if we generate any shared_ptr of FrameGraphObject, it will cause unintentional destruction.
DeviceTexture2DDX12* DeviceTexture2DDX12::BuildDeviceTexture2DDX12(const std::string& name, ID3D12Resource* tex, ResourceUsage usage)
{
	D3D12_RESOURCE_DESC desc = tex->GetDesc();
	DataFormatType format = static_cast<DataFormatType>(desc.Format);
	u32 bp = static_cast<u32>(TextureBindPosition::TBP_None);
	if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		bp |= TextureBindPosition::TBP_DS;
	}
	else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
	{
		bp |= TextureBindPosition::TBP_RT;
	}
	else if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
	{
		bp |= TextureBindPosition::TBP_Shader;
	}
	auto fg_tex = new FrameGraphTexture2D(name, format, static_cast<u32>(desc.Width), desc.Height, bp);
	fg_tex->SetUsage(usage);
	auto ret = new DeviceTexture2DDX12(tex, fg_tex);
	fg_tex->SetDeviceObject(ret);

	return ret;
}

DeviceTexture2DDX12::DeviceTexture2DDX12(ID3D12Resource* deviceTex, FrameGraphTexture2D* tex)
	: DeviceTextureDX12(tex)
{
	D3D12_RESOURCE_DESC desc = deviceTex->GetDesc();
	assert(desc.Width == tex->GetWidth());
	assert(desc.Height == tex->GetHeight());
	assert(desc.MipLevels == tex->GetMipLevelNum());

	m_deviceResPtr = deviceTex;
	DeviceCom12Ptr device;
	deviceTex->GetDevice(IID_PPV_ARGS(device.GetAddressOf()));
	const auto TBP = tex->GetBindPosition();

	if (tex->GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
	{
		CreateStaging(device.Get(), desc);
	}

	// Create views of the texture.
	if ((TBP & TBP_Shader) && ((TBP & TBP_DS) == 0))
	{
		CreateSRView(device.Get(), desc);
	}

	if (TBP & TBP_RT)
	{
		CreateRTView(device.Get(), desc);
	}

	if (TBP & TBP_DS)
	{
		if (TBP & TBP_Shader)
		{
			CreateDSSRView(device.Get(), desc);
		}
		else
		{
			CreateDSView(device.Get(), desc);
		}
	}

	if (tex->GetUsage() == ResourceUsage::RU_SHADER_OUTPUT)
	{
		CreateUAView(device.Get(), desc);
	}

	// Generate mipmaps if requested.
	//if (tex->WantAutoGenerateMips() && m_srv)
	{
		///TODO
	}
}

DeviceTexture2DDX12::DeviceTexture2DDX12(ID3D12Device* device, FrameGraphTexture2D* tex)
	: DeviceTextureDX12(tex)
{
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = tex->GetWidth();
	desc.Height = tex->GetHeight();
	desc.DepthOrArraySize = 1;
	desc.MipLevels = static_cast<u16>(tex->GetMipLevelNum());
	desc.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
	desc.SampleDesc.Count = tex->GetSampCount();
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	const auto TBP = tex->GetBindPosition();

	if (TBP & TBP_DS)
	{
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	CD3DX12_HEAP_PROPERTIES properties(D3D12_HEAP_TYPE_DEFAULT);
	HR(device->CreateCommittedResource(
		&properties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		GetResourceState(),
		&optClear,
		IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())
	));
	m_gpuVirtualAddress = m_deviceResPtr->GetGPUVirtualAddress();

	if (tex->GetUsage() == ResourceUsage::RU_IMMUTABLE && (TBP & TBP_Shader) && tex->GetData())
	{
		// In order to copy CPU memory data into our default buffer, we need to create
		// an intermediate upload heap. 
		CD3DX12_HEAP_PROPERTIES heap_staging_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		HR(device->CreateCommittedResource(
			&heap_staging_properties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_stagingResPtr.GetAddressOf())));

		SyncCPUToGPU();
	}

	if (tex->GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
	{
		CreateStaging(device, desc);
	}

	// Create views of the texture.
	if ((TBP & TBP_Shader) && ((TBP & TBP_DS) == 0))
	{
		CreateSRView(device, desc);
	}

	if (TBP & TBP_RT)
	{
		CreateRTView(device, desc);
	}

	if (TBP & TBP_DS)
	{
		if (TBP & TBP_Shader)
		{
			CreateDSSRView(device, desc);
		}
		else
		{
			CreateDSView(device, desc);
		}
	}
}

void DeviceTexture2DDX12::SyncCPUToGPU()
{
	auto res = m_frameGraphObjPtr.lock_down<FrameGraphResource>();
	ResourceUsage usage = res->GetUsage();

	if (usage == ResourceUsage::RU_IMMUTABLE && res->GetData())
	{
		auto cmdList = RendererContext::GetCurrentRender()->CommandList();

		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = res->GetData();
		subResourceData.RowPitch = res->GetNumBytes();
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
		// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
		// the intermediate upload heap data will be copied to mBuffer.
		auto transitionToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
			GetResourceState(),
			D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResourceBarrier(1, &transitionToCopyDest);
		UpdateSubresources<1>(cmdList, m_deviceResPtr.Get(), m_stagingResPtr.Get(), 0, 0, 1, &subResourceData);
		auto transitionBack = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &transitionBack);
		SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
	}
}

void DeviceTexture2DDX12::SyncGPUToCPU()
{
	assert(m_stagingResPtr);

	RendererContext::GetCurrentRender()->ResetCommandList();
	auto device = RendererContext::GetCurrentRender()->GetDevice();
	// The footprint may depend on the device of the resource, but we assume there is only one device.
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
	auto srcDesc = m_deviceResPtr->GetDesc();
	device->GetCopyableFootprints(&srcDesc, 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);

	auto state = GetResourceState();
	RendererContext::GetCurrentRender()->TransitionResource(this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	auto dstLocation = CD3DX12_TEXTURE_COPY_LOCATION(m_stagingResPtr.Get(), PlacedFootprint);
	auto srcLocation = CD3DX12_TEXTURE_COPY_LOCATION(m_deviceResPtr.Get(), 0);
	RendererContext::GetCurrentRender()->CommandList()->CopyTextureRegion(
		&dstLocation, 0, 0, 0,
		&srcLocation, nullptr);
	RendererContext::GetCurrentRender()->TransitionResource(this, state);
	HR(RendererContext::GetCurrentRender()->CommandList()->Close());

	ID3D12CommandList* cmdLists[] = { RendererContext::GetCurrentRender()->CommandList() };
	RendererContext::GetCurrentRender()->CommandQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	RendererContext::GetCurrentRender()->FlushCommandQueue();

	void* memory;
	auto range = CD3DX12_RANGE(0, GetFrameGraphResource()->GetNumBytes());
	m_stagingResPtr->Map(0, &range, &memory);

	// Copy from staging texture to CPU memory.
	auto fgTex2 = m_frameGraphObjPtr.lock_down<FrameGraphTexture2D>();
	const auto pitch = fgTex2->GetWidth() * fgTex2->GetElementSize();
	FrameGraphResource::CopyPitched2(fgTex2->GetHeight(), PlacedFootprint.Footprint.RowPitch, (u8*)memory, pitch, fgTex2->GetData());

	auto range2 = CD3DX12_RANGE(0, 0);
	m_stagingResPtr->Unmap(0, &range2);
}

shared_ptr<FrameGraphTexture2D> DeviceTexture2DDX12::GetFrameGraphTexture2D()
{
	auto ptr = FrameGraphObject();
	forward::FrameGraphObject* p_obj = ptr.get();
	auto p = dynamic_cast<FrameGraphTexture2D*>(p_obj);

	return shared_ptr<FrameGraphTexture2D>(p);
}

void DeviceTexture2DDX12::CreateStaging(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	assert(!m_stagingResPtr);
	assert(m_deviceResPtr);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
	auto srcDesc = m_deviceResPtr->GetDesc();
	device->GetCopyableFootprints(&srcDesc, 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);

	// Create a readback buffer large enough to hold all texel data
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_READBACK);

	// Readback buffers must be 1-dimensional, i.e. "buffer" not "texture2d"
	D3D12_RESOURCE_DESC ResourceDesc = {};
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// we have to make sure the staging resource is not smaller than render target, 
	// otherwise, we'll encounter error when using CopyTextureRegion in SyncGPUToCPU
	ResourceDesc.Width = PlacedFootprint.Footprint.RowPitch * tx.Height;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HR(device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_stagingResPtr)));
}

void DeviceTexture2DDX12::CreateRTView(ID3D12Device* device, const D3D12_RESOURCE_DESC& /*tx*/)
{
	m_rtvHandle = RendererContext::GetCurrentRender()->AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GetDeviceResource().Get(), nullptr, m_rtvHandle);
}

void DeviceTexture2DDX12::CreateDSView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = tx.Format;
	dsvDesc.Texture2D.MipSlice = 0;

	m_dsvHandle = RendererContext::GetCurrentRender()->AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(GetDeviceResource().Get(), &dsvDesc, m_dsvHandle);
}

void DeviceTexture2DDX12::CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	m_srvHandle = RendererContext::GetCurrentRender()->AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tx.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = tx.MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	device->CreateShaderResourceView(m_deviceResPtr.Get(), &srvDesc, m_srvHandle);
}

void DeviceTexture2DDX12::CreateUAView(ID3D12Device* /*device*/, const D3D12_RESOURCE_DESC& /*tx*/)
{

}

void DeviceTexture2DDX12::CreateDSSRView(ID3D12Device* /*device*/, const D3D12_RESOURCE_DESC& /*tx*/)
{

}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceTexture2DDX12::GetDepthStencilViewHandle()
{
	return m_dsvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceTexture2DDX12::GetRenderTargetViewHandle()
{
	return m_rtvHandle;
}