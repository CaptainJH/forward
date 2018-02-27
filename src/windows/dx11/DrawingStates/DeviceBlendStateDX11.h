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

	private:
		// Conversions from FrameGraph values to DX11 values.
		static const D3D11_BLEND msMode[];
		static const D3D11_BLEND_OP msOperation[];
	};
}