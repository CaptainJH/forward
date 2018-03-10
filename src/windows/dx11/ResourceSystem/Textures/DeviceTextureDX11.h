//***************************************************************************************
// DeviceTextureDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx11/ResourceSystem/DeviceResourceDX11.h"

namespace forward
{
	class DeviceTextureDX11 : public DeviceResourceDX11
	{
	public:

		ShaderResourceViewComPtr	GetSRView() const;
		UnorderedAccessViewComPtr	GetUAView() const;

	protected:
		ShaderResourceViewComPtr		m_srv;
		UnorderedAccessViewComPtr		m_uav;
	};
}
