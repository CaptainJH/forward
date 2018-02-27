//***************************************************************************************
// DeviceTexture2DDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceTextureDX11.h"

namespace forward
{
	class DeviceTexture2DDX11 : public DeviceTextureDX11
	{
	public:

		ResourceType	GetType() override;

		DepthStencilStateComPtr		GetDSView() const;
		RenderTargetViewComPtr		GetRTView() const;

	protected:
		DepthStencilStateComPtr		m_dsv;
		RenderTargetViewComPtr		m_rtv;

	};
}