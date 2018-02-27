//***************************************************************************************
// DeviceIndexBufferDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceBufferDX11.h"

namespace forward
{
	class DeviceIndexBufferDX11 : public DeviceBufferDX11
	{
	public:

		ResourceType	GetType() override;
	};
}
