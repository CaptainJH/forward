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

void DeviceTextureDX11::CopyPitched2(u32 numRows, u32 srcRowPitch, const u8* srcData, u32 dstRowPitch, u8* dstData)
{
	if (srcRowPitch == dstRowPitch)
	{
		// The memory is contiguous.
		memcpy(dstData, srcData, dstRowPitch * numRows);
	}
	else
	{
		// Padding was added to each row of the texture, so we must
		// copy a row at a time to compensate for differing pitches.
		auto numRowBytes = std::min(srcRowPitch, dstRowPitch);
		auto srcRow = srcData;
		auto dstRow = dstData;
		for (auto row = 0U; row < numRows; ++row)
		{
			memcpy(dstRow, srcRow, numRowBytes);
			srcRow += srcRowPitch;
			dstRow += dstRowPitch;
		}
	}
}


void DeviceTextureDX11::CopyPitched3(u32 numRows, u32 numSlices, u32 srcRowPitch, u32 srcSlicePitch, const u8* srcData, 
	u32 dstRowPitch, u32 dstSlicePitch, u8* dstData)
{
	if (srcRowPitch == dstRowPitch && srcSlicePitch == dstSlicePitch)
	{
		// The memory is contiguous.
		memcpy(dstData, srcData, dstSlicePitch * numSlices);
	}
	else
	{
		// Padding was added to each row and/or slice of the texture, so
		// we must copy the data to compensate for differing pitches.
		auto numRowBytes = std::min(srcRowPitch, dstRowPitch);
		auto srcSlice = srcData;
		auto dstSlice = dstData;
		for (auto slice = 0U; slice < numSlices; ++slice)
		{
			auto srcRow = srcSlice;
			auto dstRow = dstSlice;
			for (auto row = 0U; row < numRows; ++row)
			{
				memcpy(dstRow, srcRow, numRowBytes);
				srcRow += srcRowPitch;
				dstRow += dstRowPitch;
			}
			srcSlice += srcSlicePitch;
			dstSlice += dstSlicePitch;
		}
	}
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