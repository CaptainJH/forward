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
	class RendererDX12;

	class DevicePipelineStateObjectDX12 : public DeviceObject
	{
	public:
		DevicePipelineStateObjectDX12(RendererDX12* render, const PipelineStateObject& pso);
		virtual ~DevicePipelineStateObjectDX12();

		ID3D12PipelineState* GetDevicePSO();

	private:
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];

		// Conversions from FrameGraph values to DX12 values.
		static i8 const* msSemantic[VA_NUM_SEMANTICS];

		PipelineStateComPtr			m_devicePSO;
		RootSignatureComPtr			m_rootSignature;
	};
}