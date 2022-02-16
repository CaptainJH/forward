//***************************************************************************************
// DeviceConstantBufferDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceBufferDX11.h"

namespace forward
{
	class ConstantBufferBase;

	class DeviceConstantBufferDX11 : public DeviceBufferDX11
	{
	public:

		DeviceConstantBufferDX11(ID3D11Device* device, ConstantBufferBase* cb);
	};
}
