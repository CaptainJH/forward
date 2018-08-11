//***************************************************************************************
// DeviceTexture2DDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX11.h"

using namespace forward;

DeviceTexture2DDX11* DeviceTexture2DDX11::BuildDeviceTexture2DDX11(const std::string& name, ID3D11Texture2D* tex, ResourceUsage usage/*=RU_IMMUTABLE*/)
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
	fg_tex->SetUsage(usage);
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
	desc.SampleDesc.Count = tex->GetSampCount();
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

	const auto TBP = tex->GetBindPosition();

	DeviceContextComPtr context;
	device->GetImmediateContext(context.GetAddressOf());
	bool autoGenMip = false;

	if (TBP & TBP_RT)
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
	else if (TBP & TBP_Shader)
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
		else // usage == ResourceUsage::RU_SHADER_OUTPUT
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
	}
	
	// Create the texture.
	Texture2DComPtr dxTexture;
	if ( tex->GetData() && desc.SampleDesc.Count == 1 )
	{
		if (tex->IsFileTexture() && autoGenMip)
		{
				desc.MipLevels = 0;
				HR(device->CreateTexture2D(&desc, nullptr, dxTexture.GetAddressOf()));

				u32 numBytes = 0;
				u32 rowBytes = 0;
				GetSurfaceInfo(tex->GetWidth(), tex->GetHeight(), tex->GetFormat(), &numBytes, &rowBytes);
				assert(numBytes <= tex->GetNumBytes());

				context->UpdateSubresource(dxTexture.Get(), 0, nullptr, tex->GetData(), rowBytes, numBytes);
		}
		else
		{
			std::unique_ptr<D3D11_SUBRESOURCE_DATA[]> initData(new (std::nothrow) D3D11_SUBRESOURCE_DATA[tex->GetMipLevelNum()]);
			assert(initData);

			u32 skipMip = 0;
			u32 twidth = 0;
			u32 theight = 0;
			u32 tdepth = 0;
			FillInitDataDX11(tex->GetWidth(), tex->GetHeight(), 1, tex->GetMipLevelNum(), 1, tex->GetFormat(), 0, tex->GetNumBytes(),
				tex->GetData(), twidth, theight, tdepth, skipMip, initData.get());

			HR(device->CreateTexture2D(&desc, initData.get(), dxTexture.GetAddressOf()));
		}
	}
	else
	{
		// multi-sampled texture can only go this branch
		HR(device->CreateTexture2D(&desc, nullptr, dxTexture.GetAddressOf()));
	}

	m_deviceResPtr = dxTexture;

	if (tex->GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
	{
		CreateStaging(device, desc);
	}

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
			CreateDSSRView(device, desc);
		}
		else
		{
			CreateDSView(device, desc);
		}
	}

	if (tex->GetUsage() == ResourceUsage::RU_SHADER_OUTPUT)
	{
		CreateUAView(device, desc);
	}

	// Generate mipmaps if requested.
	if (autoGenMip && m_srv)
	{
		context->GenerateMips(m_srv.Get());
	}
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

void DeviceTexture2DDX11::CreateDSView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Format = static_cast<DXGI_FORMAT>(GetFrameGraphTexture2D()->GetFormat());
	desc.ViewDimension = tx.SampleDesc.Count == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
	desc.Flags = 0;
	desc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(GetDXTexture2DPtr(), &desc, m_dsv.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateDSSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = GetDepthSRVFormat(static_cast<DXGI_FORMAT>(GetFrameGraphTexture2D()->GetFormat()));
	desc.ViewDimension = tx.SampleDesc.Count == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(GetDXTexture2DPtr(), &desc, m_srv.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateRTView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = tx.Format;
	desc.ViewDimension = tx.SampleDesc.Count == 1 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
	desc.Texture2D.MipSlice = 0;

	HR(device->CreateRenderTargetView(GetDXTexture2DPtr(), &desc, m_rtv.GetAddressOf()));
}

void DeviceTexture2DDX11::CreateStaging(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width				= tx.Width;
	desc.Height				= tx.Height;
	desc.MipLevels			= tx.MipLevels;
	desc.ArraySize			= tx.ArraySize;
	desc.Format				= tx.Format;
	desc.SampleDesc.Count	= tx.SampleDesc.Count;
	desc.SampleDesc.Quality = tx.SampleDesc.Quality;
	desc.Usage				= D3D11_USAGE_STAGING;
	desc.BindFlags			= D3D11_BIND_NONE;
	desc.CPUAccessFlags		= D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags			= D3D11_RESOURCE_MISC_NONE;

	Texture2DComPtr stagingTex;
	HR(device->CreateTexture2D(&desc, nullptr, stagingTex.GetAddressOf()));
	m_stagingResPtr = stagingTex;
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

void DeviceTexture2DDX11::SyncGPUToCPU(ID3D11DeviceContext* context)
{
	if (PrepareForSync())
	{
		// Copy from GPU memory to staging texture.
		ID3D11Resource* dxTexture = GetDXTexture2DPtr();
		context->CopyResource(m_stagingResPtr.Get(), dxTexture);

		// Map the staging texture.
		D3D11_MAPPED_SUBRESOURCE sub;
		HR(context->Map(m_stagingResPtr.Get(), 0, D3D11_MAP_READ, 0, &sub));

		// Copy from staging texture to CPU memory.
		auto fgRes = m_frameGraphObjPtr.lock_down<FrameGraphResource>();
		if (GetFrameGraphResource()->GetType() == FGOT_TEXTURE1)
		{
			memcpy(fgRes->GetData(), sub.pData, fgRes->GetNumBytes());
		}
		else if (GetFrameGraphResource()->GetType() == FGOT_TEXTURE2)
		{
			auto fgTex2 = m_frameGraphObjPtr.lock_down<FrameGraphTexture2D>();
			CopyPitched2(fgTex2->GetHeight(), sub.RowPitch, (u8*)sub.pData, fgTex2->GetWidth() * fgTex2->GetElementSize(), fgTex2->GetData());
		}
		//else if (GetType() == ResourceType::RT_TEXTURE3D)
		//{
		//	/// TODO
		//}

		context->Unmap(m_stagingResPtr.Get(), 0);
	}
}