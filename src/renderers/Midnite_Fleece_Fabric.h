#pragma once
#include "FrameGraph/Renderer.h"
#include "RHI/SceneData.h"
#include "AutodeskStandardSurface.h"

namespace forward
{
	//struct Midnite_Fleece_Fabric_CB2
	//{
	//	//displacementshader displacementshader1;
	//	Vector3f		offset;
	//	float				scale;
	//	float node_multiply_9_in2 = 2.0f;
	//	int node_image_color3_2_layer; int padding0; int padding1;
	//	Vector3f node_image_color3_2_default;
	//	int node_image_color3_2_uaddressmode = 2;
	//	int node_image_color3_2_vaddressmode = 2;
	//	int node_image_color3_2_filtertype = 1;
	//	int node_image_color3_2_framerange;
	//	int node_image_color3_2_frameoffset;
	//	int node_image_color3_2_frameendaction;
	//	Vector2f node_image_color3_2_uv_scale = {1.0f, 1.0f}; int padding2;
	//	Vector2f node_image_color3_2_uv_offset;
	//	int node_image_vector3_12_layer; int padding3;
	//	Vector3f node_image_vector3_12_default;
	//	int node_image_vector3_12_uaddressmode = 2;
	//	int node_image_vector3_12_vaddressmode = 2;
	//	int node_image_vector3_12_filtertype = 1;
	//	int node_image_vector3_12_framerange;
	//	int node_image_vector3_12_frameoffset;
	//	int node_image_vector3_12_frameendaction;
	//	Vector2f node_image_vector3_12_uv_scale = { 1.0f, 1.0f }; int padding4;
	//	Vector2f node_image_vector3_12_uv_offset;
	//	int node_image_vector3_10_layer; int padding5;
	//	Vector3f node_image_vector3_10_default;
	//	int node_image_vector3_10_uaddressmode = 2;
	//	int node_image_vector3_10_vaddressmode = 2;
	//	int node_image_vector3_10_filtertype = 1;
	//	int node_image_vector3_10_framerange;
	//	int node_image_vector3_10_frameoffset;
	//	int node_image_vector3_10_frameendaction;
	//	Vector2f node_image_vector3_10_uv_scale = { 1.0f, 1.0f }; int padding6;
	//	Vector2f node_image_vector3_10_uv_offset;
	//	int node_extract_11_index;
	//	int node_normalmap_space;
	//	float node_normalmap_scale = 1;
	//	float node_mix_3_fg = 0.915f;
	//	float node_mix_3_bg;
	//	float SR_Midnite_Fleece_Fabric_base = 0.8f;
	//	float SR_Midnite_Fleece_Fabric_diffuse_roughness;
	//	float SR_Midnite_Fleece_Fabric_metalness;
	//	float SR_Midnite_Fleece_Fabric_specular = 1.0f; int padding7;
	//	Vector3f SR_Midnite_Fleece_Fabric_specular_color = { 1.0f, 1.0f, 1.0f };
	//	float SR_Midnite_Fleece_Fabric_specular_IOR = 1.5f;
	//	float SR_Midnite_Fleece_Fabric_specular_anisotropy;
	//	float SR_Midnite_Fleece_Fabric_specular_rotation;
	//	float SR_Midnite_Fleece_Fabric_transmission; int padding8;
	//	Vector3f SR_Midnite_Fleece_Fabric_transmission_color = { 1.0f, 1.0f, 1.0f };
	//	float SR_Midnite_Fleece_Fabric_transmission_depth;
	//	Vector3f SR_Midnite_Fleece_Fabric_transmission_scatter;
	//	float SR_Midnite_Fleece_Fabric_transmission_scatter_anisotropy;
	//	float SR_Midnite_Fleece_Fabric_transmission_dispersion;
	//	float SR_Midnite_Fleece_Fabric_transmission_extra_roughness;
	//	float SR_Midnite_Fleece_Fabric_subsurface; int padding9;
	//	Vector3f SR_Midnite_Fleece_Fabric_subsurface_color = { 1.0f, 1.0f, 1.0f }; int padding10;
	//	Vector3f SR_Midnite_Fleece_Fabric_subsurface_radius = { 1.0f, 1.0f, 1.0f };
	//	float SR_Midnite_Fleece_Fabric_subsurface_scale = 1.0f;
	//	float SR_Midnite_Fleece_Fabric_subsurface_anisotropy;
	//	float SR_Midnite_Fleece_Fabric_sheen; int padding11[2];
	//	Vector3f SR_Midnite_Fleece_Fabric_sheen_color = { 1.0f, 1.0f, 1.0f };
	//	float SR_Midnite_Fleece_Fabric_sheen_roughness = 0.3f;
	//	float SR_Midnite_Fleece_Fabric_coat;
	//	Vector3f SR_Midnite_Fleece_Fabric_coat_color = {1.0f, 1.0f, 1.0f};
	//	float SR_Midnite_Fleece_Fabric_coat_roughness = 0.1f;
	//	float SR_Midnite_Fleece_Fabric_coat_anisotropy;
	//	float SR_Midnite_Fleece_Fabric_coat_rotation;
	//	float SR_Midnite_Fleece_Fabric_coat_IOR = 1.5f;
	//	float SR_Midnite_Fleece_Fabric_coat_affect_color;
	//	float SR_Midnite_Fleece_Fabric_coat_affect_roughness;
	//	float SR_Midnite_Fleece_Fabric_thin_film_thickness;
	//	float SR_Midnite_Fleece_Fabric_thin_film_IOR = 1.5f;
	//	float SR_Midnite_Fleece_Fabric_emission;
	//	Vector3f SR_Midnite_Fleece_Fabric_emission_color = { 1.0f, 1.0f, 1.0f };
	//	Vector3f SR_Midnite_Fleece_Fabric_opacity = { 1.0f, 1.0f, 1.0f };
	//	bool SR_Midnite_Fleece_Fabric_thin_walled;
	//};

	class Midnite_Fleece_Fabric final : public RendererBase
	{
	public:
		Midnite_Fleece_Fabric(SceneData& sd)
		{
			mVS = forward::make_shared<VertexShader>("StandSurface_VS", L"Midnite_Fleece_Fabric_vs", "main");
			mPS = forward::make_shared<PixelShader>("StandSurface_PS", L"Midnite_Fleece_Fabric_ps", "main");

			mSamp = forward::make_shared<SamplerState>("Env_Samp");
			mSamp->mode[0] = SamplerState::Mode::WRAP;
			mSamp->mode[1] = SamplerState::Mode::WRAP;
			mSamp->mode[2] = SamplerState::Mode::WRAP;

			FeedWithSceneData(sd);
		}

		shared_ptr<VertexShader> mVS;
		shared_ptr<PixelShader> mPS;

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> mMeshBuffers;
		shared_ptr<SamplerState> mSamp;
		shared_ptr<Texture2D> envRadianceTex;
		shared_ptr<Texture2D> envIrradianceTex;

		shared_ptr<Texture2D> image_color3_2;
		shared_ptr<Texture2D> image_vec3_12;
		shared_ptr<Texture2D> image_vec3_10;

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

						pso.m_PSState.m_shaderResources[2] = image_color3_2;
						pso.m_PSState.m_shaderResources[3] = image_vec3_12;
						pso.m_PSState.m_shaderResources[4] = image_vec3_10;

						// setup geometry
						if (p.second && p.second->GetNumElements())
							pso.m_IAState.m_indexBuffer = p.second;
						pso.m_IAState.m_topologyType = p.second->GetPrimitiveType();

						pso.m_IAState.m_vertexBuffers[0] = p.first;
						pso.m_IAState.m_vertexLayout = p.first->GetVertexFormat();

						pso.m_RSState.m_rsState.frontCCW = true;

						// setup render states
						pso.m_OMState.m_renderTargetResources[0] = r.GetDefaultRT();
						pso.m_OMState.m_depthStencilResource = r.GetDefaultDS();
					},
					[&](CommandList& r) {
						r.DrawIndexed(p.second->GetNumElements());
					}, m_renderPassVec.empty() ? RenderPass::OF_DEFAULT : RenderPass::OF_NO_CLEAN
			));

		}
	};
}
