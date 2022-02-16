//***************************************************************************************
// DeviceTextureCubeDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTextureCubeDX11.h"

using namespace forward;

DeviceTextureCubeDX11::DeviceTextureCubeDX11(ID3D11Device* device, TextureCube* tex)
	: DeviceTextureDX11(tex)
{
	D3D11_TEXTURE2D_DESC desc;
	// Specify the texture description.
	desc.Width = tex->GetWidth();
	desc.Height = tex->GetHeight();
	desc.MipLevels = tex->GetMipLevelNum();
	desc.ArraySize = 6;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	const auto TBP = tex->GetBindPosition();
	assert(TBP == TBP_Shader);
	desc.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	DeviceContextComPtr context;
	device->GetImmediateContext(context.GetAddressOf());
	bool autoGenMip = false;

	auto usage = tex->GetUsage();
	if (usage == ResourceUsage::RU_IMMUTABLE)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}
	else if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
	{
		// DX11 does not allow a cube map to be a dynamic-update resource.
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}
	else if (usage == ResourceUsage::RU_SHADER_OUTPUT)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}

	if (tex->WantAutoGenerateMips() && CanAutoGenerateMips(tex, device))
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		autoGenMip = true;
	}

	// Create the texture.
	Texture2DComPtr dxTexture;
	if (tex->GetData() && tex->IsFileTexture())
	{
		if (autoGenMip)
		{
			// TODO
		}
		else
		{
			std::unique_ptr<D3D11_SUBRESOURCE_DATA[]> initData(new (std::nothrow) D3D11_SUBRESOURCE_DATA[tex->GetMipLevelNum() * 6]);
			assert(initData);

			u32 skipMip = 0;
			u32 twidth = 0;
			u32 theight = 0;
			u32 tdepth = 0;
			FillInitDataDX11(tex->GetWidth(), tex->GetHeight(), 1, tex->GetMipLevelNum(), 6, tex->GetFormat(), 0, tex->GetNumBytes(),
				tex->GetData(), twidth, theight, tdepth, skipMip, initData.get());

			HR(device->CreateTexture2D(&desc, initData.get(), dxTexture.GetAddressOf()));
		}
	}

	m_deviceResPtr = dxTexture;

	// Create views of the texture.
	CreateSRView(device, desc);
	if (tex->GetUsage() == ResourceUsage::RU_SHADER_OUTPUT)
	{
		CreateUAView(device, desc);
	}

	//// Create a staging texture if requested.
	//if (tex->GetUsage() != ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
	//{
	//	CreateStaging(device, desc);
	//}

	// Generate mipmaps
	if (autoGenMip && m_srv)
	{
		context->GenerateMips(m_srv.Get());
	}

}

ID3D11Texture2D* DeviceTextureCubeDX11::GetDXTexture2DPtr()
{
	if (m_deviceResPtr)
	{
		return static_cast<ID3D11Texture2D*>(m_deviceResPtr.Get());
	}

	return nullptr;
}

shared_ptr<TextureCube> DeviceTextureCubeDX11::GetFrameGraphTextureCube()
{
	auto ptr = GraphicsObject();
	forward::GraphicsObject* p_obj = ptr.get();
	auto p = dynamic_cast<TextureCube*>(p_obj);

	return shared_ptr<TextureCube>(p);
}

void DeviceTextureCubeDX11::CreateSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = tx.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = tx.MipLevels;

	HR(device->CreateShaderResourceView(GetDXTexture2DPtr(), &desc, m_srv.GetAddressOf()));
}

void DeviceTextureCubeDX11::CreateUAView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = tx.Format;
	desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MipSlice = 0;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize = tx.ArraySize;

	HR(device->CreateUnorderedAccessView(GetDXTexture2DPtr(), &desc, m_uav.GetAddressOf()));
}

void DeviceTextureCubeDX11::SyncCPUToGPU()
{

}