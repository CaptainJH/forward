//***************************************************************************************
// DeviceSamplerStateDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceDrawingStateDX11.h"
#include "render/FrameGraph/PipelineStateObjects.h"

namespace forward
{
	class DeviceSamplerStateDX11 : public DeviceDrawingStateDX11
	{
	public:
		
		DeviceSamplerStateDX11(ID3D11Device* device, SamplerState* sampState);
		virtual ~DeviceSamplerStateDX11();

		ID3D11SamplerState* GetSamplerStateDX11();
		shared_ptr<SamplerState> GetSamplerState();

	private:
		// Conversions from framegraph values to DX11 values.
		static D3D11_FILTER const msFilter[];
		static D3D11_TEXTURE_ADDRESS_MODE const msMode[];
		static D3D11_COMPARISON_FUNC const msComparison[];
	};
}