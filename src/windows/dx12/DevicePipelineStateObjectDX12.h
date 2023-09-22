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
	struct RTPipelineStateObject;
	class DeviceDX12;

	struct DevicePipelineStateObjectHelper
	{
		template<class T, i32 N, i32 K>
		static void CollectBindingInfo(T shaderStageState, std::array<u32, N>& usedRegisterCBV, std::array<u32, K>& usedRegisterSRV)
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
					if (tex.IsGpuWritable()) continue;
					auto register_index = tex.GetBindPoint();
					assert(usedRegisterSRV[register_index] == 0
						&& tex.GetBindCount() == 1);
					++usedRegisterSRV[register_index];
				}
				for (auto& tex : deviceShader->GetTextureArrays())
				{
					if (tex.IsGpuWritable()) continue;
					auto register_index = tex.GetBindPoint();
					assert(usedRegisterSRV[register_index] == 0
						&& tex.GetBindCount() == 1);
					++usedRegisterSRV[register_index];
				}
				for (auto& buf : deviceShader->GetByteAddressBuffers())
				{
					if (buf.IsGpuWritable()) continue;
					auto register_index = buf.GetBindPoint();
					assert(usedRegisterSRV[register_index] == 0
						&& buf.GetBindCount() == 1);
					++usedRegisterSRV[register_index];
				}
				for (auto& buf : deviceShader->GetStructuredBuffers())
				{
					if (buf.IsGpuWritable()) continue;
					auto register_index = buf.GetBindPoint();
					assert(usedRegisterSRV[register_index] == 0
						&& buf.GetBindCount() == 1);
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

		template<class T, i32 N, i32 K>
		static void CollectBindingInfo(T shaderStageState, std::array<u32, N>& usedRegisterCBV, std::array<u32, K>& usedRegisterSRV, std::array<u32, 8>& usedRegisterUAV)
		{
			CollectBindingInfo(shaderStageState, usedRegisterCBV, usedRegisterSRV);
			if (shaderStageState.m_shader)
			{
				auto deviceShader = device_cast<ShaderDX12*>(shaderStageState.m_shader);
				for (auto& tex : deviceShader->GetTextures())
				{
					if (tex.IsGpuWritable())
					{
						auto register_index = tex.GetBindPoint();
						assert(usedRegisterUAV[register_index] == 0
							&& tex.GetBindCount() == 1);
						++usedRegisterUAV[register_index];
					}
				}
				for (auto i = 0U; i < shaderStageState.m_uavShaderRes.size(); ++i)
				{
					if (shaderStageState.m_uavShaderRes[i])
					{
						assert(usedRegisterUAV[i] == 1);
						++usedRegisterUAV[i];
					}
				}
			}
		}

		template<i32 N>
		static bool CheckBindingInfo(std::array<u32, N>& usedRegisterArray)
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
	};

	class DevicePipelineStateObjectDX12 : public DeviceObject
	{
		friend class CommandListDX12;
	public:
		DevicePipelineStateObjectDX12(DeviceDX12* d, PipelineStateObject& pso);
		~DevicePipelineStateObjectDX12() override;

		ID3D12PipelineState* GetDevicePSO();
		bool IsEmptyRootParams() const;

	private:
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];

		PipelineStateComPtr			m_devicePSO;
		RootSignatureComPtr			m_rootSignature;
		PipelineStateObject&			m_pso;

	private:
		void BuildRootSignature(ID3D12Device* device);
		void ConfigRasterizerState(D3D12_RASTERIZER_DESC& desc) const;
		void ConfigBlendState(D3D12_BLEND_DESC& desc) const;
		void ConfigDepthStencilState(D3D12_DEPTH_STENCIL_DESC& desc) const;
		std::vector<CD3DX12_STATIC_SAMPLER_DESC> ConfigStaticSamplerStates() const;
	};

	class DeviceRTPipelineStateObjectDX12 : public DeviceObject
	{
		friend class CommandListDX12;
	public:
		DeviceRTPipelineStateObjectDX12(DeviceDX12* d, RTPipelineStateObject& rtPSO);
		~DeviceRTPipelineStateObjectDX12() override;

		ID3D12StateObject* GetDeviceRTPSO();

	private:
		RTPipelineStateComPtr			m_devicePSO;
		RootSignatureComPtr				m_raytracingGlobalRootSignature;
		RootSignatureComPtr				m_raytracingLocalRootSignature;
		DeviceResCom12Ptr					m_bottomLevelAccelerationStructure;
		DeviceResCom12Ptr					m_topLevelAccelerationStructure;
		RTPipelineStateObject&			m_rtPSO;

	private:
		void BuildAccelerationStructures(DeviceDX12* device);
		void BuildRootSignature(DeviceDX12* device);
		void BuildRaytracingPipelineStateObject(DeviceDX12* device);
		void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void BuildShaderTables(DeviceDX12* device);

		static void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc);
	};
}