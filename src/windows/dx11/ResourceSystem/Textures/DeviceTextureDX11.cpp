//***************************************************************************************
// DeviceTextureDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceTextureDX11.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"

using namespace forward;

DeviceTextureDX11::DeviceTextureDX11(FrameGraphTexture* tex)
	: DeviceResourceDX11(tex)
{}

ShaderResourceViewComPtr DeviceTextureDX11::GetSRView() const
{
	return m_srv;
}

UnorderedAccessViewComPtr DeviceTextureDX11::GetUAView() const
{
	return m_uav;
}

bool DeviceTextureDX11::CanAutoGenerateMips(FrameGraphTexture* tex, ID3D11Device* device)
{
	bool autogen = false;

	if (tex->GetMipLevelNum() == 1)
	{
		// See if format is supported for auto-gen mipmaps (varies by feature level)
		DXGI_FORMAT format = static_cast<DXGI_FORMAT>(tex->GetFormat());
		u32 fmtSupport = 0;
		HR(device->CheckFormatSupport(format, &fmtSupport));
		if (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN)
		{
			// 10level9 feature levels do not support auto-gen mipgen for volume textures
			if (/*(resDim != D3D11_RESOURCE_DIMENSION_TEXTURE3D)
				||*/ (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_0))
			{
				autogen = true;
			}
		}
	}

	return autogen;
}