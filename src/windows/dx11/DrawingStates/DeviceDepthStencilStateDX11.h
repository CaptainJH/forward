//***************************************************************************************
// DeviceDepthStencilStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"
#include "render/FrameGraph/PipelineStateObjects.h"

namespace forward
{
	class DeviceDepthStencilStateDX11 : public DeviceDrawingStateDX11
	{
	public:

		DeviceDepthStencilStateDX11(ID3D11Device* device, DepthStencilState* dsState);
		virtual ~DeviceDepthStencilStateDX11();

		ID3D11DepthStencilState * GetDepthStencilStateDX11();
		DepthStencilState* GetDepthStencilState();

		void Bind(ID3D11DeviceContext* deviceContext);

	private:
		// Conversions from FrameGraph values to DX11 values.
		static D3D11_DEPTH_WRITE_MASK const msWriteMask[];
		static D3D11_COMPARISON_FUNC const msComparison[];
		static D3D11_STENCIL_OP const msOperation[];
	};
}