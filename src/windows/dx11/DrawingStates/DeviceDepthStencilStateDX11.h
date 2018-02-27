//***************************************************************************************
// DeviceDepthStencilStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"

namespace forward
{
	class DeviceDepthStencilStateDX11 : public DeviceDrawingStateDX11
	{
	public:

		ID3D11DepthStencilState * GetDepthStencilState();

	private:
		// Conversions from FrameGraph values to DX11 values.
		static D3D11_DEPTH_WRITE_MASK const msWriteMask[];
		static D3D11_COMPARISON_FUNC const msComparison[];
		static D3D11_STENCIL_OP const msOperation[];
	};
}