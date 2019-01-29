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
		void Bind(ID3D12GraphicsCommandList* commandList);

	private:
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];

		PipelineStateComPtr			m_devicePSO;
		RootSignatureComPtr			m_rootSignature;
		const PipelineStateObject&	m_pso;

		// Conversions from FrameGraph values to DX12 values.
		static D3D12_FILL_MODE const msFillMode[];
		static D3D12_CULL_MODE const msCullMode[];
		static D3D12_BLEND const msBlendMode[];
		static D3D12_BLEND_OP const msBlendOp[];
		static D3D12_DEPTH_WRITE_MASK const msWriteMask[];
		static D3D12_COMPARISON_FUNC const msComparison[];
		static D3D12_STENCIL_OP const msStencilOp[];

		static D3D12_PRIMITIVE_TOPOLOGY_TYPE Convert2DX12TopologyType(PrimitiveTopologyType topo);

	private:
		void BuildRootSignature(ID3D12Device* device);
		void ConfigRasterizerState(D3D12_RASTERIZER_DESC& desc) const;
		void ConfigBlendState(D3D12_BLEND_DESC& desc) const;
		void ConfigDepthStencilState(D3D12_DEPTH_STENCIL_DESC& desc) const;

	};
}