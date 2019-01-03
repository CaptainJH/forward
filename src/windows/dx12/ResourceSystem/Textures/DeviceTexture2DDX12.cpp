//***************************************************************************************
// DeviceTexture2DDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX12.h"

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

void DeviceTexture2DDX12::CreateStaging(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{

}

void DeviceTexture2DDX12::CreateRTView(ID3D12Device* device, const D3D12_RESOURCE_DESC& tx)
{
	HR(device->CreateRenderTargetView(GetDeviceResource().Get(), nullptr, ))
}