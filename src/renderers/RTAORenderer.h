#pragma once
#include "ReferencePTRenderer.h"

namespace forward
{
	class RTAORenderer : public RendererBase
	{
	public:
		struct RayGenCB
		{
			f32 gAORadius;
			u32  gFrameCount;
			f32 gMinT;
			u32  gNumRays;
		};

		RTAORenderer(SceneData& sd)
			: m_sceneData(sd)
		{
		}

		shared_ptr<ConstantBuffer<RayGenCB>> m_cb;
		shared_ptr<Texture2D> m_uavRT;
		shared_ptr<Texture2D> m_posWorld;
		shared_ptr<Texture2D> m_normalWorld;

		shared_ptr<ConstantBuffer<u32>> m_cb_accumulation;
		shared_ptr<Texture2D> m_accumulationTex;

		void SetupRenderPass(Device& d, bool useAccumulation=false)
		{
			m_renderPassVec.emplace_back(RenderPass(
				[&](RenderPassBuilder& /*builder*/, RTPipelineStateObject& pso) {
					pso.FeedWithSceneData(m_sceneData);
					pso.m_maxPayloadSizeInByte = 64;

					m_cb = make_shared<ConstantBuffer<RayGenCB>>("RTAO_CB");
					pso.m_rtState.m_constantBuffers[0] = m_cb;

					auto rt = d.GetDefaultRT();
					m_uavRT = make_shared<Texture2D>("RTAO_UAV_RT", DF_R8G8B8A8_UNORM, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
					m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
					pso.m_rtState.m_uavShaderRes[0] = m_uavRT;

					pso.m_rtState.m_shaderResources[0] = m_posWorld;
					pso.m_rtState.m_shaderResources[1] = m_normalWorld;

					// setup shaders
					pso.m_rtState.m_shader = make_shared<RaytracingShaders>("RTAO_Shader", L"RTAO");

					pso.m_rtState.m_rayGenShaderTable = forward::make_shared<ShaderTable>("RTAO_RayGenShaderTable", L"AoRayGen");
					pso.m_rtState.m_hitShaderTable = forward::make_shared<ShaderTable>("RTAO_HitGroupShaderTable", L"HitGroup_AoAnyHit");
					pso.m_rtState.m_missShaderTable = forward::make_shared<ShaderTable>("RTAO_MissShaderTable", L"AoMiss");
				},
				[&, useAccumulation](CommandList& cmdList) {
					auto& rtPSO = m_renderPassVec.front().GetPSO<RTPipelineStateObject>();
					cmdList.DispatchRays(rtPSO);
					if (!useAccumulation)
						cmdList.CopyResource(*d.GetCurrentSwapChainRT(), *m_uavRT);
				}
			));

			if (useAccumulation)
			{
				m_renderPassVec.emplace_back(RenderPass(
					[&](RenderPassBuilder& /*builder*/, ComputePipelineStateObject& pso) {

						m_cb_accumulation = make_shared<ConstantBuffer<u32>>("RTAO_ACCUMULATION_CB");
						pso.m_CSState.m_constantBuffers[0] = m_cb_accumulation;

						pso.m_CSState.m_uavShaderRes[0] = m_uavRT;
						auto rt = d.GetDefaultRT();
						m_accumulationTex = make_shared<Texture2D>("RTAO_ACCUMULATION_UAV_OUTPUT", DF_R32G32B32A32_FLOAT, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
						m_accumulationTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
						pso.m_CSState.m_uavShaderRes[1] = m_accumulationTex;

						// setup shaders
						pso.m_CSState.m_shader = make_shared<ComputeShader>("RTAO_ACCUMULATION_Shader", L"SimpleAccumulation", "AccumulationMain");
					},
					[&](CommandList& cmdList) {
						u32 x = m_uavRT->GetWidth() / 8;
						u32 y = m_uavRT->GetHeight() / 8;
						cmdList.Dispatch(x, y, 1);
						cmdList.CopyResource(*d.GetCurrentSwapChainRT(), *m_uavRT);
					}
				));
			}
		}

	private:
		SceneData& m_sceneData;
	};
}