//***************************************************************************************
// DeviceTexture2DDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

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
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())
	));

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

void DeviceTexture2DDX12::SyncCPUToGPU()
{

}

shared_ptr<FrameGraphTexture2D> DeviceTexture2DDX12::GetFrameGraphTexture2D()
{
	auto ptr = FrameGraphObject();
	forward::FrameGraphObject* p_obj = ptr.get();
	auto p = dynamic_cast<FrameGraphTexture2D*>(p_obj);

	return shared_ptr<FrameGraphTexture2D>(p);
}

void DeviceTexture2DDX12::CreateStaging(ID3D12Device* /*device*/, const D3D12_RESOURCE_DESC& /*tx*/)
{

}

void DeviceTexture2DDX12::CreateRTView(ID3D12Device* device, const D3D12_RESOURCE_DESC& /*tx*/)
{
	m_rtvHandle = RendererContext::GetCurrentRender()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GetDeviceResource().Get(), nullptr, m_rtvHandle);
}

void DeviceTexture2DDX12::CreateDSView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = tx.Format;
	dsvDesc.Texture2D.MipSlice = 0;

	m_dsvHandle = RendererContext::GetCurrentRender()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(GetDeviceResource().Get(), &dsvDesc, m_dsvHandle);
}

void DeviceTexture2DDX12::CreateSRView(ID3D12Device* /*device*/, const D3D12_RESOURCE_DESC& /*tx*/)
{

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