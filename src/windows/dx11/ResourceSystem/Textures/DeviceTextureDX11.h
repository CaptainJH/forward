//***************************************************************************************
// DeviceTextureDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx11/ResourceSystem/DeviceResourceDX11.h"

namespace forward
{
	class FrameGraphTexture;

	class DeviceTextureDX11 : public DeviceResourceDX11
	{
	public:

		DeviceTextureDX11(FrameGraphTexture* tex);

		ShaderResourceViewComPtr	GetSRView() const;
		UnorderedAccessViewComPtr	GetUAView() const;

	protected:
		ShaderResourceViewComPtr		m_srv;
		UnorderedAccessViewComPtr		m_uav;

		static void CopyPitched2(u32 numRows, u32 srcRowPitch, const u8* srcData, u32 dstRowPitch, u8* dstData);
		static void CopyPitched3(u32 numRows, u32 numSlices, u32 srcRowPitch, u32 srcSlicePitch, const u8* srcData, u32 dstRowPitch, u32 dstSlicePitch, u8* dstData);
	};
}
