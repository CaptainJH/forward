#pragma once
#include "FrameGraph/Renderer.h"
#include "RHI/SceneData.h"

namespace forward
{
	class RasterGBufferRenderer final : public RendererBase
	{
		struct CB
		{
			float4x4 ViewProjMatrix;
			float4x4 WorldMatrix;
		};

	public:
		RasterGBufferRenderer(SceneData& sd)
		{
			mVS = forward::make_shared<VertexShader>("SimpleAlbedo_VS", L"BasicShader", "VSMain_P_N_T_UV");
			mPS = forward::make_shared<PixelShader>("SimpleAlbedo_PS", L"DXR_Tutorials", "Tutorial_3_PS");
			mSamp = forward::make_shared<SamplerState>("SimpleAlbedo_Samp");

			FeedWithSceneData(sd);
		}

		shared_ptr<VertexShader> mVS;
		shared_ptr<PixelShader> mPS;
		Vector<shared_ptr<ConstantBuffer<CB>>> mCBs;
		shared_ptr<SamplerState> mSamp;

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> mMeshBuffers;
		Vector<shared_ptr<Texture2D>> mAlbedoTexs;
		Vector<shared_ptr<Texture2D>> mNormalTexs;
		Vector<float4x4> mInstMatrix;

		shared_ptr<Texture2D> m_gBuffer_Pos;
		shared_ptr<Texture2D> m_gBuffer_Normal;
		shared_ptr<Texture2D> m_gBuffer_Diffuse;

		shared_ptr<Texture2D> m_rt_color;
		shared_ptr<Texture2D> m_depth;

		void FeedWithSceneData(SceneData& sd)
		{
			for (auto idx = 0U; idx < sd.mInstances.size(); ++idx)
			{
				auto& inst = sd.mInstances[idx];
				auto& material = sd.mMaterials[inst.materialId];
				if (material.normalTexName.empty() || material.roughnessMetalnessTexName.empty())
					continue;
				auto texId = material.materialData.baseColorTexIdx;
				auto norId = material.materialData.normalTexIdx;
				mAlbedoTexs.emplace_back(sd.mTextures[texId]);
				mNormalTexs.emplace_back(sd.mTextures[norId]);
				auto& geo = sd.mMeshData[inst.meshId];
				mMeshBuffers.emplace_back(std::make_pair(geo.m_VB, geo.m_IB));
				mInstMatrix.emplace_back(inst.mat);

				std::stringstream ss;
				ss << "instance_" << idx;
				mCBs.emplace_back(forward::make_shared<ConstantBuffer<CB>>(ss.str()));
			}
		}

		void SetupRenderPass(Device& r)
		{
			assert(mMeshBuffers.size() == mAlbedoTexs.size());
			for (auto idx = 0U; idx < mMeshBuffers.size(); ++idx)
			{
				auto& p = mMeshBuffers[idx];
				auto& albedoTex = mAlbedoTexs[idx];
				auto& normalTex = mNormalTexs[idx];
				auto& cb = mCBs[idx];
				const bool isLast = idx == mMeshBuffers.size() - 1;
				m_renderPassVec.push_back(RenderPass(
					[&](RenderPassBuilder& /*builder*/, RasterPipelineStateObject& pso) {
						// setup shaders
						pso.m_VSState.m_shader = mVS;
						pso.m_PSState.m_shader = mPS;

						pso.m_PSState.m_shaderResources[0] = albedoTex;
						pso.m_PSState.m_shaderResources[1] = normalTex;
						pso.m_PSState.m_samplers[0] = mSamp;

						// setup geometry
						if (p.second && p.second->GetNumElements())
							pso.m_IAState.m_indexBuffer = p.second;
						pso.m_IAState.m_topologyType = p.second->GetPrimitiveType();

						pso.m_IAState.m_vertexBuffers[0] = p.first;
						pso.m_IAState.m_vertexLayout = p.first->GetVertexFormat();

						// setup constant buffer
						pso.m_VSState.m_constantBuffers[0] = cb;

						// setup render states
						pso.m_OMState.m_renderTargetResources[0] = m_rt_color;
						pso.m_OMState.m_renderTargetResources[1] = m_gBuffer_Pos;
						pso.m_OMState.m_renderTargetResources[2] = m_gBuffer_Normal;
						pso.m_OMState.m_depthStencilResource = m_depth;
					},
					[&, isLast](CommandList& cmdList) {
						cmdList.DrawIndexed(p.second->GetNumElements());
						if (isLast)
							cmdList.CopyResource(*r.GetCurrentSwapChainRT(), *m_rt_color);
					},
					m_renderPassVec.empty() ? RenderPass::OF_DEFAULT : RenderPass::OF_NO_CLEAN
				));
			}
		}

		void updateConstantBuffer(float4x4 viewMat, float4x4 projMat)
		{
			for (auto idx = 0U; idx < mInstMatrix.size(); ++idx)
			{
				*mCBs[idx] = {
					.ViewProjMatrix = viewMat * projMat,
					.WorldMatrix = mInstMatrix[idx]
				};
			}
		}
	};
}
