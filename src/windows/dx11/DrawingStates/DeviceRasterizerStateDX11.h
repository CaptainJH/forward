//***************************************************************************************
// DeviceRasterizerStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"
#include "render/FrameGraph/PipelineStateObjects.h"

namespace forward
{
	class DeviceRasterizerStateDX11 : public DeviceDrawingStateDX11
	{
	public:

		DeviceRasterizerStateDX11(ID3D11Device* device, RasterizerState* rsState);
		virtual ~DeviceRasterizerStateDX11();

		ID3D11RasterizerState * GetRasterizerStateDX11();
		shared_ptr<RasterizerState> GetRasterizerState();

		void Bind(ID3D11DeviceContext* deviceContext);

	private:
		// Conversions from FrameGraph values to DX11 values.
		static D3D11_FILL_MODE const msFillMode[];
		static D3D11_CULL_MODE const msCullMode[];
	};
}