//***************************************************************************************
// DeviceResourceDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "render/ResourceSystem/DeviceResource.h"
#include "dx11/dx11Util.h"

namespace forward
{
	class DeviceResourceDX11 : public DeviceResource
	{
	public:
		DeviceResourceDX11(forward::FrameGraphObject* obj);

		virtual ~DeviceResourceDX11();

		DeviceResComPtr		GetDeviceResource();

		u32					GetEvictionPriority() override;
		void				SetEvictionPriority(u32 EvictionPriority) override;

	protected:
		DeviceResComPtr		m_deviceResPtr;
	};
}
