#include "utilities/Application.h"
#include "renderers/RTAORenderer.h"
#include "renderers/SimpleAlbedo.h"
#include "utilities/Log.h"

using namespace forward;

class AOBaker : public Application
{
	struct CB
	{
		float4x4 WorldMatrix;
		float4x4 InverseTransposeWorldMatrix;
	};

public:
	AOBaker(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"AOBaker";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override { m_pause = !m_pause; }

private:
	std::unique_ptr<RenderPass> m_renderPass;

	shared_ptr<Texture2D> m_posTex;
	shared_ptr<Texture2D> m_normalTex;
	shared_ptr<Texture2D> m_colorTex;
	shared_ptr<ConstantBuffer<CB>> m_cb;
	shared_ptr<RTAORenderer> m_rtaoRenderer;
	shared_ptr<SimpleAlbedoRenderer> m_aoRenderer;
	SceneData m_sceneData;

	u32 m_frames = 0U;
	u32 m_accumulatedFrames = 0U;
	bool m_pause = false;
};

void AOBaker::UpdateScene(f32 dt) 
{
	*m_cb = {
		.WorldMatrix = m_sceneData.mInstances.front().mat,
		.InverseTransposeWorldMatrix = m_sceneData.mInstances.front().mat.inverse().transpose(),
	};
	m_rtaoRenderer->Update(dt); 

	mFPCamera.UpdateViewMatrix();
	m_aoRenderer->Update(dt);

	m_accumulatedFrames = m_pause ? m_accumulatedFrames + 1 : 0U;
}

void AOBaker::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass.get());
	m_rtaoRenderer->DrawEffect(&fg);
	m_aoRenderer->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool AOBaker::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	m_sceneData = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);
	auto& geo = m_sceneData.mMeshData.front();
	auto albedoTex = m_sceneData.mTextures[1];
	//auto normalTex = m_sceneData.mTextures[2];
	m_renderPass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {

			m_cb = make_shared<ConstantBuffer<CB>>("AOBaker_CB");
			builder.GetRenderPass()->m_vs.m_constantBuffers[0] = m_cb;

			// setup shaders
			pso.m_VSState.m_shader = make_shared<VertexShader>("AOBaker_VS", L"AOBaker", "VSMain");
			pso.m_PSState.m_shader = make_shared<PixelShader>("AOBaker_PS", L"AOBaker", "PSMain");

			builder.GetRenderPass()->m_ps.m_shaderResources[0] = albedoTex;
			//pso.m_PSState.m_shaderResources[1] = normalTex;
			pso.m_PSState.m_samplers[0] = make_shared<SamplerState>("AOBaker_Samp");

			// setup geometry
			builder.GetRenderPass()->m_ia_params.m_indexBuffer = geo.m_IB;
			builder.GetRenderPass()->m_ia_params.m_topologyType = geo.m_IB->GetPrimitiveType();
			builder.GetRenderPass()->m_ia_params.m_vertexBuffers[0] = geo.m_VB;
			pso.m_IAState.m_vertexLayout = geo.m_VB->GetVertexFormat();

			const u32 size = 1024;
			m_colorTex = make_shared<Texture2D>("Color_OUTPUT", DF_R8G8B8A8_UNORM, size, size,
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);
			m_posTex = make_shared<Texture2D>("Pos_OUTPUT", DF_R32G32B32A32_FLOAT, size, size,
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);
			m_normalTex = make_shared<Texture2D>("Normal_OUTPUT", DF_R32G32B32A32_FLOAT, size, size,
				TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);
			// setup render states
			builder.GetRenderPass()->m_om_params.m_renderTargetResources[0] = m_colorTex;
			builder.GetRenderPass()->m_om_params.m_renderTargetResources[1] = m_posTex;
			builder.GetRenderPass()->m_om_params.m_renderTargetResources[2] = m_normalTex;
			builder.GetRenderPass()->m_om_params.m_depthStencilResource = m_pDevice->GetDefaultDS();

			pso.m_rsState.cullMode = RasterizerState::CULL_NONE;
		},
		[&](CommandList& cmdList) {
			cmdList.DrawIndexed(geo.m_IB->GetNumElements());
			cmdList.CopyResource(*m_pDevice->GetCurrentSwapChainRT(), *m_colorTex);
	});

	m_rtaoRenderer = make_shared<RTAORenderer>(m_sceneData);
	m_rtaoRenderer->m_posWorld = m_posTex;
	m_rtaoRenderer->m_normalWorld = m_normalTex;
	m_rtaoRenderer->SetupRenderPass(*m_pDevice, true);

	m_rtaoRenderer->mUpdateFunc = [&](f32) {
		*m_rtaoRenderer->m_cb = {
			.gAORadius = 1.0f,
			.gFrameCount = ++m_frames,
			.gMinT = 0.001f,
			.gNumRays = 2
		};
		if (m_rtaoRenderer->m_cb_accumulation)
			*m_rtaoRenderer->m_cb_accumulation = m_accumulatedFrames;
		};

	m_aoRenderer = make_shared<SimpleAlbedoRenderer>(m_sceneData);
	m_aoRenderer->SetupRenderPass(*m_pDevice);
	m_aoRenderer->m_renderPassVec.front().m_ps.m_shaderResources[0] = m_rtaoRenderer->m_uavRT;

	m_aoRenderer->mUpdateFunc = [&](f32 dt) {
		auto object2WorldMatrix = m_sceneData.mInstances.front().mat;
		static f32 rotAngle = 0.0f;
		if (!m_pause)
			rotAngle += dt * 0.001f;
		float4x4 rotM;
		rotM.rotate(float3(0, rotAngle, 0));
		*m_aoRenderer->mCBs[0] = {
			.ViewProjMatrix = mFPCamera.GetViewMatrix() * mFPCamera.GetProjectionMatrix(),
			.WorldMatrix = object2WorldMatrix * rotM,
		};
		};

	return true;
}

FORWARD_APPLICATION_MAIN(AOBaker, 1024, 1024);
