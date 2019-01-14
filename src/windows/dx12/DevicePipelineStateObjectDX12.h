//***************************************************************************************
// DevicePipelineStateObjectDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/DeviceObject.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"
#include "render/ShaderSystem/FrameGraphShader.h"
#include "dx12/dx12Util.h"


namespace forward
{
	struct PipelineStateObject;

	class DevicePipelineStateObjectDX12 : public DeviceObject
	{
	public:
		DevicePipelineStateObjectDX12(ID3D12Device* device, const PipelineStateObject& pso);
		virtual ~DevicePipelineStateObjectDX12();

	private:
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];

		// Conversions from FrameGraph values to DX11 values.
		static i8 const* msSemantic[VA_NUM_SEMANTICS];

		PipelineStateComPtr			m_devicePSO;
	};
}