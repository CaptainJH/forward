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

	private:
		SceneData& m_sceneData;
	};
}