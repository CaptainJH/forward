#pragma once
#include "ReferencePTRenderer.h"

namespace forward
{
	class RTLambertRenderer : public RendererBase
	{
	public:
		struct RayGenCB
		{
			float3 gLightIntensity;
			float gMinT;      // Min distance to start a ray to avoid self-occlusion
			float3 gLightPos;
			u32 gFrameCount;
		};

		RTLambertRenderer(SceneData& sd)
			: m_sceneData(sd)
		{
		}

		shared_ptr<ConstantBuffer<RayGenCB>> m_cb;
		shared_ptr<Texture2D> m_uavRT;
		shared_ptr<Texture2D> m_posWorld;
		shared_ptr<Texture2D> m_normalWorld;
		shared_ptr<Texture2D> m_diffuse;
		shared_ptr<Texture2D> m_envTex;

		shared_ptr<ConstantBuffer<u32>> m_cb_accumulation;
		shared_ptr<Texture2D> m_accumulationTex;

		void SetupRenderPass(Device& d)
		{
			auto rt = d.GetDefaultRT();
			const auto w = rt->GetWidth();
			const auto h = rt->GetHeight();
			m_renderPassVec.emplace_back(RenderPass(
				[&, w, h](RenderPassBuilder& /*builder*/, RTPipelineStateObject& pso) {
					pso.FeedWithSceneData(m_sceneData);

					m_cb = make_shared<ConstantBuffer<RayGenCB>>("RTLambert_CB");
					pso.m_rtState.m_constantBuffers[0] = m_cb;

					m_uavRT = make_shared<Texture2D>("RTLambert_UAV_RT", DF_R8G8B8A8_UNORM, w, h, TextureBindPosition::TBP_Shader);
					m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_rtState.m_uavShaderRes[0] = m_uavRT;

					pso.m_rtState.m_shaderResources[0] = m_posWorld;
					pso.m_rtState.m_shaderResources[1] = m_normalWorld;
					pso.m_rtState.m_shaderResources[2] = m_diffuse;

					// setup shaders
					pso.m_rtState.m_shader = make_shared<RaytracingShaders>("RTLambert_Shader", L"RTLambertShader");

					pso.m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RTLambert_RayGenShaderTable", L"LambertShadowsRayGen");
					pso.m_rtState.m_hitShaderTable = forward::make_shared<ShaderTable>("RTLambert_HitGroupShaderTable",
						Vector<WString>{ L"HitGroup_ShadowClosestHit", L"HitGroup_ShadowAnyHit" });
					pso.m_rtState.m_missShaderTable = make_shared<ShaderTable>("RTLambert_MissShaderTable", L"ShadowMiss");
				},
				[&](CommandList& cmdList) {
					auto& rtPSO = m_renderPassVec.front().GetPSO<RTPipelineStateObject>();
					cmdList.DispatchRays(rtPSO);
				}
			));

			m_renderPassVec.emplace_back(RenderPass(
				[&](RenderPassBuilder& /*builder*/, ComputePipelineStateObject& pso) {

					m_cb_accumulation = make_shared<ConstantBuffer<u32>>("RTLambert_ACCUMULATION_CB");
					pso.m_CSState.m_constantBuffers[0] = m_cb_accumulation;

					pso.m_CSState.m_uavShaderRes[0] = m_uavRT;
					auto rt = d.GetDefaultRT();
					m_accumulationTex = make_shared<Texture2D>("RTLambert_ACCUMULATION_UAV_OUTPUT", DF_R32G32B32A32_FLOAT, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
					m_accumulationTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_CSState.m_uavShaderRes[1] = m_accumulationTex;

					// setup shaders
					pso.m_CSState.m_shader = make_shared<ComputeShader>("RTLambert_ACCUMULATION_Shader", L"SimpleAccumulation", "AccumulationMain");
				},
				[&, w, h](CommandList& cmdList) {
					const u32 x = w / 8;
					const u32 y = h / 8;
					cmdList.Dispatch(x, y, 1);
					cmdList.CopyResource(*d.GetCurrentSwapChainRT(), *m_uavRT);
				}
			));
		}

		void SetupRenderPassWithGI(Device& d)
		{
			auto rt = d.GetDefaultRT();
			const auto w = rt->GetWidth();
			const auto h = rt->GetHeight();
			m_renderPassVec.emplace_back(RenderPass(
				[&, w, h](RenderPassBuilder& /*builder*/, RTPipelineStateObject& pso) {
					pso.FeedWithSceneData(m_sceneData);

					m_cb = make_shared<ConstantBuffer<RayGenCB>>("RTLambert_CB");
					pso.m_rtState.m_constantBuffers[0] = m_cb;

					m_envTex = make_shared<Texture2D>("u_envRadiance", L"Lights/san_giuseppe_bridge_split.hdr");

					m_uavRT = make_shared<Texture2D>("RTLambert_UAV_RT", DF_R8G8B8A8_UNORM, w, h, TextureBindPosition::TBP_Shader);
					m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_rtState.m_uavShaderRes[0] = m_uavRT;

					pso.m_rtState.m_shaderResources[0] = m_posWorld;
					pso.m_rtState.m_shaderResources[1] = m_normalWorld;
					pso.m_rtState.m_shaderResources[2] = m_diffuse;
					//pso.m_rtState.m_shaderResources[3] = m_envTex;

					//m_materials = make_shared<StructuredBuffer<SceneData::MaterialData>>("RTLambert_MaterialDataBuffer", (u32)m_sceneData.mMaterials.size());
					//for (auto i = 0U; i < m_sceneData.mMaterials.size(); ++i)
					//	(*m_materials)[i] = m_sceneData.mMaterials[i].materialData;

					// setup shaders
					pso.m_rtState.m_shader = make_shared<RaytracingShaders>("RTLambert_Shader", L"RTLambertGIShader");

					//auto& newlyAddedStageVB = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
					//	.m_space = Shader::VertexDataSpace });
					//for (auto& m : m_sceneData.mMeshData)
					//	newlyAddedStageVB.m_shaderResources.emplace_back(m.m_VB);
					//auto& newlyAddedStageIB = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
					//	.m_space = Shader::IndexDataSpace });
					//for (auto& m : m_sceneData.mMeshData)
					//	newlyAddedStageIB.m_shaderResources.emplace_back(m.m_IB);
					//auto& newlyAddedStageTex = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
					//	.m_space = Shader::TextureSpace });
					//for (auto& t : m_sceneData.mTextures)
					//	newlyAddedStageTex.m_shaderResources.emplace_back(t);
					//pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
					//	.m_space = Shader::MaterialDataSpace,
					//	.m_shaderResources = { m_materials },
					//	});
					//pso.m_rtState.m_samplers[0] = make_shared<SamplerState>("RefPT_Sampler0");

					pso.m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RTLambert_RayGenShaderTable", L"SimpleDiffuseGIRayGen");
					pso.m_rtState.m_hitShaderTable = forward::make_shared<ShaderTable>("RTLambert_HitGroupShaderTable",
						Vector<WString>{ L"HitGroup_ShadowClosestHit", L"HitGroup_ShadowAnyHit"/*, L"HitGroupIndirect_IndirectClosestHit", L"HitGroupIndirect_IndirectAnyHit"*/ });
					pso.m_rtState.m_missShaderTable = forward::make_shared<ShaderTable>("RTLambert_MissShaderTable",
						Vector<WString>{ L"ShadowMiss"/*, L"IndirectMiss"*/ });
				},
				[&](CommandList& cmdList) {
					auto& rtPSO = m_renderPassVec.front().GetPSO<RTPipelineStateObject>();
					cmdList.DispatchRays(rtPSO);
				}
			));

			m_renderPassVec.emplace_back(RenderPass(
				[&](RenderPassBuilder& /*builder*/, ComputePipelineStateObject& pso) {

					m_cb_accumulation = make_shared<ConstantBuffer<u32>>("RTLambert_ACCUMULATION_CB");
					pso.m_CSState.m_constantBuffers[0] = m_cb_accumulation;

					pso.m_CSState.m_uavShaderRes[0] = m_uavRT;
					auto rt = d.GetDefaultRT();
					m_accumulationTex = make_shared<Texture2D>("RTLambert_ACCUMULATION_UAV_OUTPUT", DF_R32G32B32A32_FLOAT, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
					m_accumulationTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_CSState.m_uavShaderRes[1] = m_accumulationTex;

					// setup shaders
					pso.m_CSState.m_shader = make_shared<ComputeShader>("RTLambert_ACCUMULATION_Shader", L"SimpleAccumulation", "AccumulationMain");
				},
				[&, w, h](CommandList& cmdList) {
					const u32 x = w / 8;
					const u32 y = h / 8;
					cmdList.Dispatch(x, y, 1);
					cmdList.CopyResource(*d.GetCurrentSwapChainRT(), *m_uavRT);
				}
			));
		}

	private:
		SceneData& m_sceneData;
		shared_ptr<StructuredBuffer<SceneData::MaterialData>> m_materials;
	};
}