//***************************************************************************************
// DeviceBlendStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"
#include "render/FrameGraph/PipelineStateObjects.h"

namespace forward
{
	class DeviceBlendStateDX11 : public DeviceDrawingStateDX11
	{
	public:
		DeviceBlendStateDX11(ID3D11Device* device, BlendState* blendState);
		virtual ~DeviceBlendStateDX11();

		ID3D11BlendState* GetBlendStateDX11();
		shared_ptr<BlendState> GetBlendState();

		void Bind(ID3D11DeviceContext* deviceContext);

	protected:

	private:
		// Conversions from FrameGraph values to DX11 values.
		static const D3D11_BLEND msMode[];
		static const D3D11_BLEND_OP msOperation[];
	};
}