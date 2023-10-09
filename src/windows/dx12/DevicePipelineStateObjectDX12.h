//***************************************************************************************
// DevicePipelineStateObjectDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/DeviceObject.h"
#include "RHI/ResourceSystem/Buffer.h"
#include "RHI/ShaderSystem/Shader.h"
#include "FrameGraph/PipelineStateObjects.h"
#include "dx12/dx12Util.h"
#include "dx12/ShaderSystem/ShaderDX12.h"


namespace forward
{
	struct BindingRanges;
	class DeviceDX12;
	class DynamicDescriptorHeapDX12;

	struct DevicePipelineStateObjectHelper
	{
		static void CollectBindingInfo(const ShaderDX12* deviceShader, BindingRanges& rangesCBV, BindingRanges& rangesSRV, BindingRanges& rangesUAV, u32 space = 0);
		static void CollectSamplerInfo(const ShaderDX12* deviceShader, BindingRanges& ranges);

		template<class R, class T>
		static bool CheckBindingResources(const R& bindingResources, const T& ranges, const i8* logPrefix, u32 space = 0)
		{
			bool ret = true;
			for (auto& r : ranges.m_ranges)
			{
				if (r.Count() >= Shader::BindlessDescriptorCount)
					continue;
				for (auto bind = r.bindStart; bind <= r.bindEnd; ++bind)
				{
					if (!bindingResources[bind])
					{
						std::stringstream ss;
						ss << logPrefix << bind << ", space" << space << " missing resource!" << std::endl;
						OutputDebugStringA(ss.str().c_str());
						ret = false;
					}
				}
			}
			return ret;
		}
		template<class R, class T>
		static bool CheckBindingResources(const R& bindingResources0, const R& bindingResources1, const T& ranges, const i8* logPrefix, u32 space=0)
		{
			bool ret = true;
			for (auto& r : ranges.m_ranges)
			{
				if (r.Count() >= Shader::BindlessDescriptorCount)
					continue;
				for (auto bind = r.bindStart; bind <= r.bindEnd; ++bind)
				{
					if (!bindingResources0[bind] && !bindingResources1[bind])
					{
						std::stringstream ss;
						ss << logPrefix << bind << ", space" << space << " missing resource!" << std::endl;
						OutputDebugStringA(ss.str().c_str());
						ret = false;
					}
				}
			}
			return ret;
		}

		template<class Samplers>
		static std::vector<CD3DX12_STATIC_SAMPLER_DESC> ConfigStaticSamplerDescs(Samplers& samplers)
		{
			std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplerDescs;

			for (auto i = 0U; i < samplers.size(); ++i)
			{
				auto samp = samplers[i];
				if (samp)
				{
					CD3DX12_STATIC_SAMPLER_DESC desc(
						i, DevicePipelineStateObjectHelper::msFilter[samp->filter],
						DevicePipelineStateObjectHelper::msAddressMode[samp->mode[0]],
						DevicePipelineStateObjectHelper::msAddressMode[samp->mode[1]],
						DevicePipelineStateObjectHelper::msAddressMode[samp->mode[2]],
						samp->mipLODBias,
						samp->maxAnisotropy,
						DevicePipelineStateObjectHelper::msComparison[samp->comparison],
						D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
						samp->minLOD,
						samp->maxLOD
					);
					samplerDescs.push_back(desc);
				}
			}
			return samplerDescs;
		}

		static std::vector<CD3DX12_STATIC_SAMPLER_DESC> ConfigStaticSamplerStates(PSOPtrUnion psoPtr);

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
		DevicePipelineStateObjectDX12(DeviceDX12* d, RasterPipelineStateObject& pso);
		DevicePipelineStateObjectDX12(DeviceDX12* d, ComputePipelineStateObject& pso);
		~DevicePipelineStateObjectDX12() override;

		ID3D12PipelineState* GetDevicePSO();
		bool IsEmptyRootParams() const;

	private:
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];

		PipelineStateComPtr			m_devicePSO;
		RootSignatureComPtr			m_rootSignature;
		PSOPtrUnion			m_psoPtr;

	private:
		void BuildRootSignature(ID3D12Device* device);
		void ConfigRasterizerState(D3D12_RASTERIZER_DESC& desc) const;
		void ConfigBlendState(D3D12_BLEND_DESC& desc) const;
		void ConfigDepthStencilState(D3D12_DEPTH_STENCIL_DESC& desc) const;
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
		Vector<DeviceResCom12Ptr>	m_bottomLevelAccelerationStructures;
		DeviceResCom12Ptr					m_topLevelAccelerationStructure;
		RTPipelineStateObject&			m_rtPSO;

	private:
		void BuildAccelerationStructures(DeviceDX12* device);
		void BuildRootSignature(DeviceDX12* device);
		void BuildRaytracingPipelineStateObject(DeviceDX12* device);
		void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void BuildShaderTables(DeviceDX12* device);
		std::unordered_map<WString, Vector<WString>> GetHitGroupsInfo() const;
		void PrepareDeviceResources(DeviceDX12* device);

		void StageDescriptorsToCache(Vector<CD3DX12_DESCRIPTOR_RANGE>& descriptorRanges);
		std::unique_ptr<DynamicDescriptorHeapDX12> m_bindlessDescriptorHeap;

		static void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc);
	};
}