//***************************************************************************************
// DeviceDrawingStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/DeviceObject.h"
#include "dx11/dx11Util.h"

namespace forward
{
	class DeviceDrawingStateDX11 : public DeviceObject
	{
	public:
		DeviceDrawingStateDX11(forward::FrameGraphObject* obj);

		DeviceObjComPtr		GetDeviceObject();

	protected:
		DeviceObjComPtr		m_deviceObjPtr;
	};
}
