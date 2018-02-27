//***************************************************************************************
// DeviceRasterizerStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"

namespace forward
{
	class DeviceRasterizerStateDX11 : public DeviceDrawingStateDX11
	{
	public:

		ID3D11RasterizerState * GetRasterizerState();

	private:
		// Conversions from FrameGraph values to DX11 values.
		static D3D11_FILL_MODE const msFillMode[];
		static D3D11_CULL_MODE const msCullMode[];
	};
}