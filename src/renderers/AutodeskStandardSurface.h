#pragma once
#include "FrameGraph/Renderer.h"
#include "RHI/SceneData.h"

namespace forward
{
	struct CB0
	{
		Matrix4f worldMat;
		Matrix4f viewProjMat;
		Matrix4f worldInverseTransMat;
	};

	struct CB1
	{
		Matrix4f		u_envMatrix = Matrix4f(
			-1.0f, 0, 0, 0, 
			0, 1.0f, 0, 0, 
			0, 0, -1.0f, 0,
			0, 0, 0, 1.0f);
		int				u_envRadianceMips = 1;
		int				u_envRadianceSamples = 16;
		int				u_refractionEnv = 0; int padding_0;
		Vector3f		u_refractionColor = Vector3f(0, 0, 0); int padding_1;
		Vector3f		u_viewPosition;
		int				u_numActiveLightSources = 1;
	};

	struct CB2
	{
		//displacementshader displacementshader1;
		Vector3f		offset;
		float				scale;
		float				 SR_default_base = 1.000000;
		Vector3f		 SR_default_base_color = Vector3f(0.800000f, 0.800000f, 0.800000f);
		float				 SR_default_diffuse_roughness = 0.000000;
		float				 SR_default_metalness = 0.000000;
		float				 SR_default_specular = 1.000000; int padding_0;
		Vector3f		 SR_default_specular_color = Vector3f(1.000000, 1.000000, 1.000000);
		float				 SR_default_specular_roughness = 0.200000f;
		float				 SR_default_specular_IOR = 1.500000;
		float				 SR_default_specular_anisotropy = 0.000000;
		float				 SR_default_specular_rotation = 0.000000;
		float				 SR_default_transmission = 0.000000;
		Vector3f		 SR_default_transmission_color = Vector3f(1.000000, 1.000000, 1.000000);
		float				 SR_default_transmission_depth = 0.000000;
		Vector3f		 SR_default_transmission_scatter = Vector3f(0.000000, 0.000000, 0.000000);
		float				 SR_default_transmission_scatter_anisotropy = 0.000000;
		float				 SR_default_transmission_dispersion = 0.000000;
		float				 SR_default_transmission_extra_roughness = 0.000000;
		float				 SR_default_subsurface = 0.000000; int padding_1;
		Vector3f		 SR_default_subsurface_color = Vector3f(1.000000, 1.000000, 1.000000); int padding_2;
		Vector3f		 SR_default_subsurface_radius = Vector3f(1.000000, 1.000000, 1.000000);
		float				 SR_default_subsurface_scale = 1.000000;
		float				 SR_default_subsurface_anisotropy = 0.000000;
		float				 SR_default_sheen = 0.000000; int padding_3[2];
		Vector3f		 SR_default_sheen_color = Vector3f(1.000000, 1.000000, 1.000000);
		float				 SR_default_sheen_roughness = 0.300000f;
		float				 SR_default_coat = 0.000000;
		Vector3f		 SR_default_coat_color = Vector3f(1.000000, 1.000000, 1.000000);
		float				 SR_default_coat_roughness = 0.100000f;
		float				 SR_default_coat_anisotropy = 0.000000;
		float				 SR_default_coat_rotation = 0.000000;
		float				 SR_default_coat_IOR = 1.500000;
		float				 SR_default_coat_affect_color = 0.000000;
		float				 SR_default_coat_affect_roughness = 0.000000;
		float				 SR_default_thin_film_thickness = 0.000000;
		float				 SR_default_thin_film_IOR = 1.500000;
		float				 SR_default_emission = 0.000000;
		Vector3f		 SR_default_emission_color = Vector3f(1.000000, 1.000000, 1.000000);
		Vector3f		 SR_default_opacity = Vector3f(1.000000, 1.000000, 1.000000);
		int				 SR_default_thin_walled = 0;
	};

	struct LightData
	{
		Vector3f direction = Vector3f(0.0f, 0.0f, 0.0f); 
		int pad0;
		Vector3f color = Vector3f(0.0f, 0.0f, 0.0f);;
		int type = 0;
		float intensity = 0;
		float pad1;
		float pad2;
		float pad3;
	};
	struct CB3
	{
		LightData u_lightData[3];
	};

	class AutodeskStandardSurface final : public RendererBase
	{
	public:
		AutodeskStandardSurface(SceneData& sd)
		{
			mVS = forward::make_shared<VertexShader>("StandSurface_VS", L"standard_surface_default_vs", "main");
			mPS = forward::make_shared<PixelShader>("StandSurface_PS", L"standard_surface_default_ps", "main");
			mCB0 = forward::make_shared<ConstantBuffer<CB0>>("CB_0");
			mCB1 = forward::make_shared<ConstantBuffer<CB1>>("CB_1");
			mCB2 = forward::make_shared<ConstantBuffer<CB2>>("CB_2");
			mCB3 = forward::make_shared<ConstantBuffer<CB3>>("CB_3");
			mSamp = forward::make_shared<SamplerState>("Env_Samp");

			FeedWithSceneData(sd);
		}

		shared_ptr<VertexShader> mVS;
		shared_ptr<PixelShader> mPS;
		shared_ptr<ConstantBuffer<CB0>> mCB0;
		shared_ptr<ConstantBuffer<CB1>> mCB1;
		shared_ptr<ConstantBuffer<CB2>> mCB2;
		shared_ptr<ConstantBuffer<CB3>> mCB3;

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> mMeshBuffers;
		shared_ptr<SamplerState> mSamp;
		shared_ptr<Texture2D> envRadianceTex;
		shared_ptr<Texture2D> envIrradianceTex;

		void FeedWithSceneData(SceneData& sd)
		{
			for (auto& geo : sd.mMeshData)
				mMeshBuffers.push_back(std::make_pair(geo.m_VB, geo.m_IB));
		}

		void SetupRenderPass(Device& r)
		{
			for (auto& p : mMeshBuffers)
				m_renderPassVec.push_back(RenderPass(
					[&](RenderPassBuilder& /*builder*/, RasterPipelineStateObject& pso) {
						// setup shaders
						pso.m_VSState.m_shader = mVS;
						pso.m_PSState.m_shader = mPS;

						pso.m_PSState.m_shaderResources[0] = envRadianceTex;
						pso.m_PSState.m_shaderResources[1] = envIrradianceTex;
						pso.m_PSState.m_samplers[0] = mSamp;

						// setup geometry
						if (p.second && p.second->GetNumElements())
							pso.m_IAState.m_indexBuffer = p.second;
						pso.m_IAState.m_topologyType = p.second->GetPrimitiveType();

						pso.m_IAState.m_vertexBuffers[0] = p.first;
						pso.m_IAState.m_vertexLayout = p.first->GetVertexFormat();

						// setup constant buffer
						pso.m_VSState.m_constantBuffers[0] = mCB0;
						pso.m_PSState.m_constantBuffers[1] = mCB1;
						pso.m_PSState.m_constantBuffers[2] = mCB2;
						pso.m_PSState.m_constantBuffers[3] = mCB3;

						auto& lightData0 = mCB3->GetTypedData()->u_lightData[0];
						lightData0.color = Vector3f(1.0f, 0.89447f, 0.56723f);
						lightData0.direction = Vector3f(0.51443f, -0.47901f, -0.71127f);
						lightData0.intensity = 2.52776f;
						lightData0.type = 1;

						pso.m_RSState.m_rsState.frontCCW = true;
						
						// setup render states
						pso.m_OMState.m_renderTargetResources[0] = r.GetDefaultRT();
						pso.m_OMState.m_depthStencilResource = r.GetDefaultDS();
					},
					[&](CommandList& cmdList) {
						cmdList.DrawIndexed(p.second->GetNumElements());
					}, m_renderPassVec.empty() ? RenderPass::OF_DEFAULT : RenderPass::OF_NO_CLEAN
					));

		}
	};
}