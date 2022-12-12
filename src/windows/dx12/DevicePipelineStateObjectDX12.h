//***************************************************************************************
// DevicePipelineStateObjectDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/DeviceObject.h"
#include "RHI/ResourceSystem/Buffer.h"
#include "RHI/ShaderSystem/Shader.h"
#include "dx12/dx12Util.h"
#include "dx12/ShaderSystem/ShaderDX12.h"


namespace forward
{
	struct PipelineStateObject;
	class DeviceDX12;

	class DevicePipelineStateObjectDX12 : public DeviceObject
	{
	public:
		DevicePipelineStateObjectDX12(DeviceDX12* d, PipelineStateObject& pso);
		virtual ~DevicePipelineStateObjectDX12();

		ID3D12PipelineState* GetDevicePSO();
		void Bind(ID3D12GraphicsCommandList* commandList);

	private:
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];

		PipelineStateComPtr			m_devicePSO;
		RootSignatureComPtr			m_rootSignature;
		PipelineStateObject&			m_pso;

		// Conversions from FrameGraph values to DX12 values.
		static D3D12_FILL_MODE const msFillMode[];
		static D3D12_CULL_MODE const msCullMode[];
		static D3D12_BLEND const msBlendMode[];
		static D3D12_BLEND_OP const msBlendOp[];
		static D3D12_DEPTH_WRITE_MASK const msWriteMask[];
		static D3D12_COMPARISON_FUNC const msComparison[];
		static D3D12_STENCIL_OP const msStencilOp[];
		static D3D12_FILTER const msFilter[];
		static D3D12_TEXTURE_ADDRESS_MODE const msAddressMode[];

		static D3D12_PRIMITIVE_TOPOLOGY_TYPE Convert2DX12TopologyType(PrimitiveTopologyType topo);

	private:
		void BuildRootSignature(ID3D12Device* device);
		void ConfigRasterizerState(D3D12_RASTERIZER_DESC& desc) const;
		void ConfigBlendState(D3D12_BLEND_DESC& desc) const;
		void ConfigDepthStencilState(D3D12_DEPTH_STENCIL_DESC& desc) const;
		std::vector<CD3DX12_STATIC_SAMPLER_DESC> ConfigStaticSamplerStates() const;

		template<class T, i32 N, i32 K>
		void collectBindingInfo(T shaderStageState, std::array<u32, N>& usedRegisterCBV, std::array<u32, K>& usedRegisterSRV) const
		{
			if (shaderStageState.m_shader)
			{
				auto deviceShader = device_cast<ShaderDX12*>(shaderStageState.m_shader);
				for (auto& cb : deviceShader->GetCBuffers())
				{
					auto register_index = cb.GetBindPoint();
					assert(usedRegisterCBV[register_index] == 0
						&& cb.GetBindCount() == 1);
					++usedRegisterCBV[register_index];
				}
				for (auto i = 0U; i < shaderStageState.m_constantBuffers.size(); ++i)
				{
					if (shaderStageState.m_constantBuffers[i])
					{
						assert(usedRegisterCBV[i] == 1);
						++usedRegisterCBV[i];
					}
				}

				for (auto& tex : deviceShader->GetTextures())
				{
					auto register_index = tex.GetBindPoint();
					assert(usedRegisterSRV[register_index] == 0
						&& tex.GetBindCount() == 1);
					++usedRegisterSRV[register_index];
				}
				for (auto i = 0U; i < shaderStageState.m_shaderResources.size(); ++i)
				{
					if (shaderStageState.m_shaderResources[i])
					{
						assert(usedRegisterSRV[i] == 1);
						++usedRegisterSRV[i];
					}
				}
			}
		}

		template<i32 N>
		bool checkBindingInfo(std::array<u32, N>& usedRegisterArray) const
		{
			const u64 ZeroCount = std::count(usedRegisterArray.begin(), usedRegisterArray.end(), 0U);
			if (ZeroCount == usedRegisterArray.size())
				return true;

			auto lastTwo = std::find(usedRegisterArray.rbegin(), usedRegisterArray.rend(), 2U);
			const u64 gapCount = std::abs(std::distance(usedRegisterArray.rbegin(), lastTwo));
			if (usedRegisterArray[0] == 2 && gapCount == ZeroCount)
				return true;

			assert(false && "something wrong with the binding info!");
			return false;
		}
	};
}