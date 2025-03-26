#pragma once
#include "RHI/FrameGraph/Renderer.h"
#include "RHI/SceneData.h"

namespace forward
{
	struct Light {
		float3 position;
		u32 type;
		float3 intensity;
		u32 pad;
	};

	struct RaytracingData
	{
		float4x4 view;
		float4x4 proj;

		f32 skyIntensity;
		u32 lightCount;
		u32 frameNumber;
		u32 maxBounces;

		f32 exposureAdjustment;
		u32 accumulatedFrames;
		BOOL enableAntiAliasing;
		f32 focusDistance;

		f32 apertureSize;
		BOOL enableAccumulation;
		f32 pad;
		f32 pad2;

		Light lights[4];
	};


	class ReferencePTRenderer final : public RendererBase
	{
	public:
		ReferencePTRenderer(SceneData& sd)
			: m_sceneData(sd)
		{
		}

		shared_ptr<ConstantBuffer<RaytracingData>> m_cb;
		shared_ptr<Texture2D> m_uavRT;

		void SetupRenderPass(Device& d)
		{
			m_renderPassVec.emplace_back(RenderPass(
				[&](RenderPassBuilder& /*builder*/, RTPipelineStateObject& pso) {
					pso.FeedWithSceneData(m_sceneData);
					pso.m_maxPayloadSizeInByte = 64;

					m_cb = make_shared<ConstantBuffer<RaytracingData>>("RefPT_CB");
					pso.m_rtState.m_constantBuffers[0] = m_cb;

					auto rt = d.GetDefaultRT();
					m_uavRT = make_shared<Texture2D>("RefPT_UAV_RT", DF_R8G8B8A8_UNORM, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
					m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_rtState.m_uavShaderRes[0] = m_uavRT;
					auto uavAccumulation = make_shared<Texture2D>("RefPT_UAV_Accumulation", DF_R32G32B32A32_FLOAT, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
					uavAccumulation->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_rtState.m_uavShaderRes[1] = uavAccumulation;

					m_materials = make_shared<StructuredBuffer<SceneData::MaterialData>>("RefPT_MaterialDataBuffer", (u32)m_sceneData.mMaterials.size());
					for (auto i = 0U; i < m_sceneData.mMaterials.size(); ++i)
						(*m_materials)[i] = m_sceneData.mMaterials[i].materialData;

					// setup shaders
					pso.m_rtState.m_shader = make_shared<RaytracingShaders>("RefPT_Shader", L"PathTracer");

					auto& newlyAddedStageVB = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
						.m_space = Shader::VertexDataSpace });
					for (auto& m : m_sceneData.mMeshData)
						newlyAddedStageVB.m_shaderResources.emplace_back(m.m_VB);
					auto& newlyAddedStageIB = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
						.m_space = Shader::IndexDataSpace });
					for (auto& m : m_sceneData.mMeshData)
						newlyAddedStageIB.m_shaderResources.emplace_back(m.m_IB);
					auto& newlyAddedStageTex = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
						.m_space = Shader::TextureSpace });
					for (auto& t : m_sceneData.mTextures)
						newlyAddedStageTex.m_shaderResources.emplace_back(t);
					pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
						.m_space = Shader::MaterialDataSpace,
						.m_shaderResources = { m_materials },
						});
					pso.m_rtState.m_samplers[0] = make_shared<SamplerState>("RefPT_Sampler0");

					pso.m_rtState.m_rayGenShaderTable = forward::make_shared<ShaderTable>("RefPT_RayGenShaderTable", Vector<WString>{ L"RayGen" });
					pso.m_rtState.m_hitShaderTable = forward::make_shared<ShaderTable>("RefPT_HitGroupShaderTable",
						Vector<WString>{ L"HitGroup_ClosestHit", L"HitGroup_AnyHit", L"HitGroupShadow_AnyHitShadow" });
					pso.m_rtState.m_missShaderTable = forward::make_shared<ShaderTable>("RefPT_MissShaderTable",
						Vector<WString>{ L"Miss", L"MissShadow" });
				},
				[&](CommandList& cmdList, RenderPass&) {
					auto& rtPSO = m_renderPassVec.front().GetPSO<RTPipelineStateObject>();
					cmdList.DispatchRays(rtPSO);
					cmdList.CopyResource(*d.GetCurrentSwapChainRT(), *m_uavRT);
				}
			));
		}

	private:
		SceneData& m_sceneData;
		shared_ptr<StructuredBuffer<SceneData::MaterialData>> m_materials;
	};
}