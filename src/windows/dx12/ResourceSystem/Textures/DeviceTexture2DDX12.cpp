//***************************************************************************************
// DeviceTexture2DDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX12.h"
#include "dx12/DeviceDX12.h"
#include "dx12/CommandQueueDX12.h"

using namespace forward;

// shouldn't generate any shared_ptr of the GraphicsObject within this function call
// because this is a very special function (create device object first, then the frame graph object)
// normally the reference count of the frame graph object is zero within this function, 
// if we generate any shared_ptr of GraphicsObject, it will cause unintentional destruction.
DeviceTexture2DDX12* DeviceTexture2DDX12::BuildDeviceTexture2DDX12(DeviceDX12& d, const std::string& name, ID3D12Resource* tex, ResourceUsage usage)
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
	auto fg_tex = new Texture2D(name, format, static_cast<u32>(desc.Width), desc.Height, bp);
	fg_tex->SetUsage(usage);
	auto ret = new DeviceTexture2DDX12(tex, fg_tex, d);
	fg_tex->SetDeviceObject(ret);

	return ret;
}

DeviceTexture2DDX12::DeviceTexture2DDX12(ID3D12Resource* deviceTex, Texture2D* tex, DeviceDX12& d)
	: DeviceTextureDX12(tex, d)
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

DeviceTexture2DDX12::DeviceTexture2DDX12(Texture2D* tex, DeviceDX12& d)
	: DeviceTextureDX12(tex, d)
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
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	const auto TBP = tex->GetBindPosition();
	D3D12_CLEAR_VALUE optClear;
	D3D12_CLEAR_VALUE* optClearPtr = nullptr;

	if (tex->IsFileTexture() && (TBP & TBP_Shader))
	{
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}

	if (TBP & TBP_DS)
	{
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		optClear.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		optClearPtr = &optClear;
	}

	auto device = m_device.GetDevice();

	CD3DX12_HEAP_PROPERTIES properties(D3D12_HEAP_TYPE_DEFAULT);
	HR(device->CreateCommittedResource(
		&properties,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&desc,
		GetResourceState(),
		optClearPtr,
		IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())
	));

	if (tex->GetUsage() == ResourceUsage::RU_IMMUTABLE && (TBP & TBP_Shader) && tex->GetData())
	{
		u64 uploadSize = 0U;
		device->GetCopyableFootprints(&desc, 0, tex->GetMipLevelNum(), 0, nullptr, nullptr, nullptr, &uploadSize);
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

	if (tex->GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
	{
		CreateStaging(device, desc);
		CreateUAView(device, desc);
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
	auto res = m_frameGraphObjPtr.lock_down<Resource>();
	auto resTex2 = dynamic_cast<Texture2D*>(res.get());
	ResourceUsage usage = res->GetUsage();

	if (usage == ResourceUsage::RU_IMMUTABLE && res->GetData())
	{
		auto cmdList = m_device.DeviceCommandList();

		// Describe the data we want to copy into the default buffer.
		std::unique_ptr<D3D12_SUBRESOURCE_DATA[]> initData(new D3D12_SUBRESOURCE_DATA[resTex2->GetMipLevelNum()]);
		assert(initData);

		u32 skipMip = 0;
		u32 twidth = 0;
		u32 theight = 0;
		u32 tdepth = 0;
		FillInitDataDX12(resTex2->GetWidth(), resTex2->GetHeight(), 1, resTex2->GetMipLevelNum(), 1, resTex2->GetFormat(), 0, resTex2->GetNumBytes(),
			resTex2->GetData(), twidth, theight, tdepth, skipMip, initData.get());

		// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
		// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
		// the intermediate upload heap data will be copied to mBuffer.
		auto transitionToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
			GetResourceState(),
			D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResourceBarrier(1, &transitionToCopyDest);
		UpdateSubresources(cmdList, m_deviceResPtr.Get(), m_stagingResPtr.Get(), 0, 0, resTex2->GetMipLevelNum(), initData.get());
		auto transitionBack = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &transitionBack);
		SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
	}
}

void DeviceTexture2DDX12::SyncGPUToCPU()
{
	assert(m_stagingResPtr);

	auto device = m_device.GetDevice();
	// The footprint may depend on the device of the resource, but we assume there is only one device.
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
	auto srcDesc = m_deviceResPtr->GetDesc();
	device->GetCopyableFootprints(&srcDesc, 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);

	auto state = GetResourceState();
	m_device.TransitionResource(this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	auto dstLocation = CD3DX12_TEXTURE_COPY_LOCATION(m_stagingResPtr.Get(), PlacedFootprint);
	auto srcLocation = CD3DX12_TEXTURE_COPY_LOCATION(m_deviceResPtr.Get(), 0);
	m_device.DeviceCommandList()->CopyTextureRegion(
		&dstLocation, 0, 0, 0,
		&srcLocation, nullptr);
	m_device.TransitionResource(this, state);
	m_device.GetDefaultQueue()->ExecuteCommandList([]() {});
	m_device.GetDefaultQueue()->Flush();

	void* memory;
	auto range = CD3DX12_RANGE(0, GetFrameGraphResource()->GetNumBytes());
	m_stagingResPtr->Map(0, &range, &memory);

	// Copy from staging texture to CPU memory.
	auto fgTex2 = m_frameGraphObjPtr.lock_down<Texture2D>();
	const auto pitch = fgTex2->GetWidth() * fgTex2->GetElementSize();
	Resource::CopyPitched2(fgTex2->GetHeight(), PlacedFootprint.Footprint.RowPitch, (u8*)memory, pitch, fgTex2->GetData());

	auto range2 = CD3DX12_RANGE(0, 0);
	m_stagingResPtr->Unmap(0, &range2);
}

shared_ptr<Texture2D> DeviceTexture2DDX12::GetTexture2D()
{
	auto ptr = GraphicsObject();
	forward::GraphicsObject* p_obj = ptr.get();
	auto p = dynamic_cast<Texture2D*>(p_obj);

	return shared_ptr<Texture2D>(p);
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

void DeviceTexture2DDX12::CreateRTView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	m_rtvHandle = m_device.AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (tx.SampleDesc.Count > 1)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = tx.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		device->CreateRenderTargetView(GetDeviceResource().Get(), &rtvDesc, m_rtvHandle);
	}
	else
	{
		device->CreateRenderTargetView(GetDeviceResource().Get(), nullptr, m_rtvHandle);
	}
}

void DeviceTexture2DDX12::CreateDSView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = GetTexture2D()->GetSampCount() > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = tx.Format;
	dsvDesc.Texture2D.MipSlice = 0;

	m_dsvHandle = m_device.AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(GetDeviceResource().Get(), &dsvDesc, m_dsvHandle);
}

void DeviceTexture2DDX12::CreateSRView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	m_srvHandle = m_device.AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tx.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = tx.MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	device->CreateShaderResourceView(m_deviceResPtr.Get(), &srvDesc, m_srvHandle);
}

void DeviceTexture2DDX12::CreateUAView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	m_uavHandle = m_device.AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = tx.Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	device->CreateUnorderedAccessView(m_deviceResPtr.Get(), nullptr, &uavDesc, m_uavHandle);
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