#pragma once
#include "FrameGraph/Effect.h"
#include "RHI/SceneData.h"

namespace forward
{
	class SimpleAlbedo final : public Effect
	{
	public:
		SimpleAlbedo(SceneData& sd, u32 geoId)
		{
			mVS = forward::make_shared<VertexShader>("SimpleAlbedo_VS", L"BasicShader", "VSMain_P_UV");
			mPS = forward::make_shared<PixelShader>("SimpleAlbedo_PS", L"BasicShader", "PSMain");
			mCB = forward::make_shared<ConstantBuffer<float4x4>>("SimpleAlbedo_CB");
			mSamp = forward::make_shared<SamplerState>("SimpleAlbedo_Samp");

			FeedWithSceneData(sd, geoId);
		}

		shared_ptr<VertexShader> mVS;
		shared_ptr<PixelShader> mPS;
		shared_ptr<ConstantBuffer<float4x4>> mCB;
		shared_ptr<SamplerState> mSamp;

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> mMeshBuffers;
		shared_ptr<Texture2D> mAlbedoTex;

		void FeedWithSceneData(SceneData& sd, u32 geoId)
		{
			//for (auto& geo : sd.mMeshData)
			//	mMeshBuffers.push_back(std::make_pair(geo.m_VB, geo.m_IB));
			auto& geo = sd.mMeshData[geoId];
			mMeshBuffers.push_back(std::make_pair(geo.m_VB, geo.m_IB));
		}

		void SetupRenderPass(Device& r)
		{
			for (auto& p : mMeshBuffers)
				m_renderPassVec.push_back(RenderPass(
					m_renderPassVec.empty() ? RenderPass::OF_DEFAULT : RenderPass::OF_NO_CLEAN,
					[&](RenderPassBuilder& /*builder*/, PipelineStateObject& pso) {
						// setup shaders
						pso.m_VSState.m_shader = mVS;
						pso.m_PSState.m_shader = mPS;
						
						pso.m_PSState.m_shaderResources[0] = mAlbedoTex;
						pso.m_PSState.m_samplers[0] = mSamp;
						
						// setup geometry
						if (p.second && p.second->GetNumElements())
							pso.m_IAState.m_indexBuffer = p.second;
						pso.m_IAState.m_topologyType = p.second->GetPrimitiveType();

						pso.m_IAState.m_vertexBuffers[0] = p.first;
						pso.m_IAState.m_vertexLayout = p.first->GetVertexFormat();
						
						// setup constant buffer
						pso.m_VSState.m_constantBuffers[0] = mCB;
						
						// setup render states
						pso.m_OMState.m_renderTargetResources[0] = r.GetDefaultRT();
						pso.m_OMState.m_depthStencilResource = r.GetDefaultDS();
					},
					[&](Device& r) {
						r.DrawIndexed(p.second->GetNumElements());
					}
					));

		}
	};
}