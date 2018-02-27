//***************************************************************************************
// DeviceBlendStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"

namespace forward
{
	class DeviceBlendStateDX11 : public DeviceDrawingStateDX11
	{
	public:

		ID3D11BlendState* GetBlendState();

	protected:

	};
}