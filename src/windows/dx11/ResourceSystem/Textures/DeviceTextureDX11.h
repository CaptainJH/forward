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
	};
}
