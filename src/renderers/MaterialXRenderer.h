#pragma once
#include "RHI/FrameGraph/Renderer.h"
#include "RHI/SceneData.h"

namespace forward
{
	class MaterialXRenderer final : public RendererBase
	{
	public:
		MaterialXRenderer(SceneData& sd, const i8* shaderName, const i8* vs, const i8* ps)
		{
			const std::string vsName = String(shaderName) + "_vs";
			const std::string psName = String(shaderName) + "_ps";
			mVS = forward::make_shared<VertexShader>(vsName.c_str(), vs);
			mPS = forward::make_shared<PixelShader>(psName.c_str(), ps);

			mSamp = forward::make_shared<SamplerState>("Sampler0");
			mSamp->mode[0] = SamplerState::Mode::WRAP;
			mSamp->mode[1] = SamplerState::Mode::WRAP;
			mSamp->mode[2] = SamplerState::Mode::WRAP;

			FeedWithSceneData(sd);
		}

		shared_ptr<VertexShader> mVS;
		shared_ptr<PixelShader> mPS;
		Vector<shared_ptr<ConstantBufferBase>> mConstantBuffers;
		Vector<shared_ptr<Texture2D>> mTextures;

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> mMeshBuffers;
		shared_ptr<SamplerState> mSamp;
		shared_ptr<Texture2D> envRadianceTex;
		shared_ptr<Texture2D> envIrradianceTex;

		void SetupRenderPass(Device& r)
		{
			for (auto& p : mMeshBuffers)
				m_renderPassVec.push_back(RenderPass(
					[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
						// setup shaders
						pso.m_VSState.m_shader = mVS;
						pso.m_PSState.m_shader = mPS;

						builder.GetRenderPass()->m_ps.m_shaderResources[0] = envRadianceTex;
						builder.GetRenderPass()->m_ps.m_shaderResources[1] = envIrradianceTex;
						pso.m_PSState.m_samplers[0] = mSamp;

						u32 index = 2;
						for (auto tex : mTextures)
							builder.GetRenderPass()->m_ps.m_shaderResources[index++] = tex;

						// setup geometry
						if (p.second && p.second->GetNumElements())
							builder.GetRenderPass()->m_ia_params.m_indexBuffer = p.second;
						builder.GetRenderPass()->m_ia_params.m_topologyType = p.second->GetPrimitiveType();

						builder.GetRenderPass()->m_ia_params.m_vertexBuffers[0] = p.first;
						pso.m_IAState.m_vertexLayout = p.first->GetVertexFormat();

						pso.m_rsState.frontCCW = true;

						// setup render states
						builder.GetRenderPass()->m_om_params.m_renderTargetResources[0] = r.GetDefaultRT();
						builder.GetRenderPass()->m_om_params.m_depthStencilResource = r.GetDefaultDS();
					},
					[&](CommandList& cmdList, RenderPass&) {
						cmdList.DrawIndexed(p.second->GetNumElements());
					}, 
					m_renderPassVec.empty() ? RenderPass::OF_DEFAULT : RenderPass::OF_NO_CLEAN
			));

		}

	private:

		void FeedWithSceneData(SceneData& sd)
		{
			for (auto& geo : sd.mMeshData)
				mMeshBuffers.push_back(std::make_pair(geo.m_VB, geo.m_IB));
		}
	};
}