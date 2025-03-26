#pragma once
#include "RHI/FrameGraph/Renderer.h"
#include "RHI/SceneData.h"

namespace forward
{
	class RasterGBufferRenderer final : public RendererBase
	{
		struct CB
		{
			float4x4 ViewProjMatrix;
			float4x4 WorldMatrix;
			float4x4 InverseTransposeWorldMatrix;
		};

	public:
		RasterGBufferRenderer(SceneData& sd, u32 width, u32 height)
		{
			mVS = make_shared<VertexShader>("SimpleAlbedo_VS", L"BasicShader", "VSMain_P_N_T_UV");
			mPS = make_shared<PixelShader>("SimpleAlbedo_PS", L"DXR_Tutorials", "Tutorial_3_PS");
			mCB1 = make_shared<ConstantBuffer<float3>>("CB1");
			mSamp = make_shared<SamplerState>("SimpleAlbedo_Samp");

			m_rt_color = make_shared<Texture2D>("RT_Color", DF_R8G8B8A8_UNORM, width, height, 
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);
			m_depth = make_shared<Texture2D>("RT_Depth", DF_D24_UNORM_S8_UINT, width, height, TextureBindPosition::TBP_DS);
			m_gBuffer_Pos = make_shared<Texture2D>("RT_gBuffer_Pos", DF_R32G32B32A32_FLOAT, width, height, 
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);
			m_gBuffer_Normal = make_shared<Texture2D>("RT_gBuffer_Normal", DF_R32G32B32A32_FLOAT, width, height, 
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);
			m_gBuffer_RoughnessMetalness = make_shared<Texture2D>("RT_gBuffer_RoughnessMetalness", DF_R32G32B32A32_FLOAT, width, height,
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);

			FeedWithSceneData(sd);
		}

		shared_ptr<VertexShader> mVS;
		shared_ptr<PixelShader> mPS;
		Vector<shared_ptr<ConstantBuffer<CB>>> mCBs;
		shared_ptr<ConstantBuffer<float3>> mCB1;
		shared_ptr<SamplerState> mSamp;

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> mMeshBuffers;
		Vector<shared_ptr<Texture2D>> mAlbedoTexs;
		Vector<shared_ptr<Texture2D>> mNormalTexs;
		Vector<shared_ptr<Texture2D>> mRoughnessMetalnessTexs;
		Vector<float4x4> mInstMatrix;

		shared_ptr<Texture2D> m_gBuffer_Pos;
		shared_ptr<Texture2D> m_gBuffer_Normal;
		shared_ptr<Texture2D> m_gBuffer_RoughnessMetalness;

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
				auto rmId = material.materialData.roughnessMetalnessTexIdx;
				mAlbedoTexs.emplace_back(sd.mTextures[texId]);
				mNormalTexs.emplace_back(sd.mTextures[norId]);
				mRoughnessMetalnessTexs.emplace_back(sd.mTextures[rmId]);
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
				auto& roughnessMetalnessTex = mRoughnessMetalnessTexs[idx];
				auto& cb = mCBs[idx];
				const bool isLast = idx == mMeshBuffers.size() - 1;
				m_renderPassVec.push_back(RenderPass(
					[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
						// setup shaders
						pso.m_VSState.m_shader = mVS;
						pso.m_PSState.m_shader = mPS;

						builder.GetRenderPass()->m_ps.m_shaderResources[0] = albedoTex;
						builder.GetRenderPass()->m_ps.m_shaderResources[1] = normalTex;
						builder.GetRenderPass()->m_ps.m_shaderResources[2] = roughnessMetalnessTex;
						pso.m_PSState.m_samplers[0] = mSamp;

						// setup geometry
						if (p.second && p.second->GetNumElements())
							builder.GetRenderPass()->m_ia_params.m_indexBuffer = p.second;
						builder.GetRenderPass()->m_ia_params.m_topologyType = p.second->GetPrimitiveType();

						builder.GetRenderPass()->m_ia_params.m_vertexBuffers[0] = p.first;
						pso.m_IAState.m_vertexLayout = p.first->GetVertexFormat();

						// setup constant buffer
						builder.GetRenderPass()->m_vs.m_constantBuffers[0] = cb;
						builder.GetRenderPass()->m_ps.m_constantBuffers[1] = mCB1;

						// setup render states
						builder.GetRenderPass()->m_om_params.m_renderTargetResources[0] = m_rt_color;
						builder.GetRenderPass()->m_om_params.m_renderTargetResources[1] = m_gBuffer_Pos;
						builder.GetRenderPass()->m_om_params.m_renderTargetResources[2] = m_gBuffer_Normal;
						builder.GetRenderPass()->m_om_params.m_renderTargetResources[3] = m_gBuffer_RoughnessMetalness;
						builder.GetRenderPass()->m_om_params.m_depthStencilResource = m_depth;
					},
					[&, isLast](CommandList& cmdList, RenderPass&) {
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
					.WorldMatrix = mInstMatrix[idx],
					.InverseTransposeWorldMatrix = mInstMatrix[idx].inverse().transpose(),
				};
			}
		}
	};
}
