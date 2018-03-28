//***************************************************************************************
// DeviceTexture2DDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX11.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"

using namespace forward;

DeviceTexture2DDX11* DeviceTexture2DDX11::BuildDeviceTexture2DDX11(const std::string& name, ID3D11Texture2D* tex)
{
	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	DataFormatType format = static_cast<DataFormatType>(desc.Format);
	u32 bp = static_cast<u32>(TextureBindPosition::TBP_None);
	if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
	{
		bp |= TextureBindPosition::TBP_DS;
	}
	else if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		bp |= TextureBindPosition::TBP_RT;
	}
	else if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		bp |= TextureBindPosition::TBP_Shader;
	}
	auto fg_tex = new FrameGraphTexture2D(name, format, desc.Width, desc.Height, bp);
	auto ret = new DeviceTexture2DDX11(tex, fg_tex);
	fg_tex->SetDeviceObject(ret);

	return ret;
}

DeviceTexture2DDX11::DeviceTexture2DDX11(ID3D11Texture2D* deviceTex, FrameGraphTexture2D* tex)
	: DeviceTextureDX11(tex)
{
	D3D11_TEXTURE2D_DESC desc;
	deviceTex->GetDesc(&desc);
	assert(desc.Width == tex->GetWidth());
	assert(desc.Height == tex->GetHeight());
	assert(desc.MipLevels == tex->GetMipLevelNum());

	m_deviceResPtr = deviceTex;
	DeviceComPtr device;
	deviceTex->GetDevice(device.GetAddressOf());
	const auto TBP = tex->GetBindPosition();

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
			CreateDSSRView(device.Get());
		}
		else
		{
			CreateDSView(device.Get());
		}
	}

	if (tex->GetUsage() == ResourceUsage::RU_SHADER_OUTPUT)
	{
		CreateUAView(device.Get(), desc);
	}

	// Generate mipmaps if requested.
	if (tex->WantAutoGenerateMips() && m_srv)
	{
		ID3D11DeviceContext* context;
		device->GetImmediateContext(&context);
		context->GenerateMips(m_srv.Get());
		context->Release();
	}
}

DeviceTexture2DDX11::DeviceTexture2DDX11(ID3D11Device* device, FrameGraphTexture2D* tex)
	: DeviceTextureDX11(tex)
{
	D3D11_TEXTURE2D_DESC desc;
	// Specify the texture description.
	desc.Width = tex->GetWidth();
	desc.Height = tex->GetHeight();
	desc.MipLevels = tex->GetMipLevelNum();
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

	const auto TBP = tex->GetBindPosition();

	if (TBP & TBP_Shader)
	{
		desc.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		ResourceUsage usage = tex->GetUsage();
		if (usage == ResourceUsage::RU_IMMUTABLE)
		{
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
		}
		else if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else // usage === ResourceUsage::RU_SHADER_OUTPUT
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
		}

		if (tex->WantAutoGenerateMips())
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
			desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}
	else if (TBP & TBP_RT)
	{
		desc.Format = static_cast<DXGI_FORMAT>(tex->GetFormat());
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;

		if (tex->GetUsage() == ResourceUsage::RU_SHADER_OUTPUT)
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		if (tex->WantAutoGenerateMips())
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}
	else if (TBP & TBP_DS)
	{
		desc.MipLevels = 1;
		desc.Format = GetDepthResourceFormat(static_cast<DXGI_FORMAT>(tex->GetFormat()));
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;

		if (TBP & TBP_Shader)
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
	}
	
	// Create the texture.
	Texture2DComPtr dxTexture;
	if (tex->GetData())
	{
		/// TODO: create texture from memory not supported now.
		HR(device->CreateTexture2D(&desc, nullptr, dxTexture.GetAddressOf()));
	}
	else
	{
		HR(device->CreateTexture2D(&desc, nullptr, dxTexture.GetAddressOf()));
	}

	m_deviceResPtr = dxTexture;

	// Create views of the texture.
	if (TBP & TBP_Shader)
	{
		CreateSRView(device, desc);
	}

	if (TBP == TBP_RT)
	{
		CreateRTView(device, desc);
	}

	if (TBP == TBP_DS)
	{
		if (TBP & TBP_Shader)
		{
			CreateDSSRView(device);
		}
		else
		{
			CreateDSView(device);
		}
	}

	if (tex->GetUsage() == ResourceUsage::RU_SHADER_OUTPUT)
	{
		CreateUAView(device, desc);
	}

	// Generate mipmaps if requested.
	if (tex->WantAutoGenerateMips() && m_srv)
	{
		ID3D11DeviceContext* context;
		device->GetImmediateContext(&context);
		context->GenerateMips(m_srv.Get());
		context->Release();
	}
}

ResourceType DeviceTexture2DDX11::GetType()
{
	return RT_TEXTURE2D;
}

ID3D11Texture2D* DeviceTexture2DDX11::GetDXTexture2DPtr()
{
	if (m_deviceResPtr)
	{
		return static_cast<ID3D11Texture2D*>(m_deviceResPtr.Get());
	}

	return nullptr;
}

shared_ptr<FrameGraphTexture2D> DeviceTexture2DDX11::GetFrameGraphTexture2D()
{
	auto ptr = FrameGraphObject();
	forward::FrameGraphObject* p_obj = ptr.get();
	auto p = dynamic_cast<FrameGraphTexture2D*>(p_obj);

	return shared_ptr<FrameGraphTexture2D>(p);
}

DepthStencilViewComPtr DeviceTexture2DDX11::GetDSView() const
{
	return m_dsv;
}

RenderTargetViewComPtr DeviceTexture2DDX11::GetRTView() const
{
	return m_rtv;
}

void DeviceTexture2DDX11::CreateSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = tx.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = tx.MipLevels;

	HR(device->CreateShaderResourceView(GetDXTexture2DPtr(), &desc, m_srv.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateUAView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = tx.Format;
	desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	HR(device->CreateUnorderedAccessView(GetDXTexture2DPtr(), &desc, m_uav.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateDSView(ID3D11Device* device)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Format = static_cast<DXGI_FORMAT>(GetFrameGraphTexture2D()->GetFormat());
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = 0;
	desc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(GetDXTexture2DPtr(), &desc, m_dsv.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateDSSRView(ID3D11Device* device)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = GetDepthSRVFormat(static_cast<DXGI_FORMAT>(GetFrameGraphTexture2D()->GetFormat()));
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(GetDXTexture2DPtr(), &desc, m_srv.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateRTView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = tx.Format;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	HR(device->CreateRenderTargetView(GetDXTexture2DPtr(), &desc, m_rtv.GetAddressOf()));
}

DXGI_FORMAT DeviceTexture2DDX11::GetDepthResourceFormat(DXGI_FORMAT depthFormat)
{
	if (depthFormat == DXGI_FORMAT_D16_UNORM)
	{
		return DXGI_FORMAT_R16_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		return DXGI_FORMAT_R24G8_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT)
	{
		return DXGI_FORMAT_R32_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
	{
		return DXGI_FORMAT_R32G8X24_TYPELESS;
	}

	Log::Get().Write(L"Invalid depth format.");
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DeviceTexture2DDX11::GetDepthSRVFormat(DXGI_FORMAT depthFormat)
{
	if (depthFormat == DXGI_FORMAT_D16_UNORM)
	{
		return DXGI_FORMAT_R16_UNORM;
	}

	if (depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT)
	{
		return DXGI_FORMAT_R32_FLOAT;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
	{
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}

	Log::Get().Write(L"Invalid depth format.");
	return DXGI_FORMAT_UNKNOWN;
}

void DeviceTexture2DDX11::SyncCPUToGPU()
{
	///TODO:
}