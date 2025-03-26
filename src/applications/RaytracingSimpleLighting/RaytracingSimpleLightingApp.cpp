#include "utilities/Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/SceneData.h"
#include "utilities/Log.h"

using namespace forward;

struct SceneConstantBuffer
{
	float4x4 projectionToWorld;
	float4 cameraPosition;
	float4 lightPosition;
	float4 lightAmbientColor;
	float4 lightDiffuseColor;
};

struct CubeConstantBuffer
{
	Vector4f albedo;
};

class RaytracingSimpleLighting : public Application
{
public:
	RaytracingSimpleLighting(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Raytracing with simple lighting!";
	}

	~RaytracingSimpleLighting()
	{
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!Application::Init())
			return false;

		m_scene = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);

		m_uavTex = make_shared<Texture2D>("UAV_Tex", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_Shader);
		m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);

		m_cb0 = make_shared<ConstantBuffer<SceneConstantBuffer>>("g_sceneCB");
		m_cb1 = make_shared<ConstantBuffer<CubeConstantBuffer>>("g_cubeCB");

		*m_cb1 = {
			.albedo = {1.0f, 1.0f, 1.0f, 1.0f}
		};

		m_rtPass = std::make_unique<RenderPass>(m_scene,
			[&](RenderPassBuilder&, RTPipelineStateObject& pso) {
				pso.m_rtState.m_constantBuffers[0] = m_cb0;
				pso.m_rtState.m_constantBuffers[1] = m_cb1;
				pso.m_rtState.m_uavShaderRes[0] = m_uavTex;

				auto& newlyAddedStageVB = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
					.m_space = Shader::VertexDataSpace });
				for (auto& m : m_scene.mMeshData)
					newlyAddedStageVB.m_shaderResources.emplace_back(m.m_VB);
				auto& newlyAddedStageIB = pso.m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
					.m_space = Shader::IndexDataSpace });
				for (auto& m : m_scene.mMeshData)
					newlyAddedStageIB.m_shaderResources.emplace_back(m.m_IB);

				pso.m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"RaytracingSimpleLighting");
				pso.m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RayGenShaderTable", L"MyRaygenShader");
				pso.m_rtState.m_hitShaderTable = make_shared<ShaderTable>("HitGroupShaderTable", L"HitGroup_MyClosestHitShader");
				pso.m_rtState.m_missShaderTable = make_shared<ShaderTable>("MissShaderTable", L"MyMissShader");

			},
			[&](CommandList& cmdList, RenderPass&) {
				cmdList.DispatchRays(m_rtPass->GetPSO<RTPipelineStateObject>());
				cmdList.CopyResource(*m_pDevice->GetCurrentSwapChainRT(), *m_uavTex);
			}
		);

		return true;
	}

protected:
	void UpdateScene(f32 dt) override
	{
		const auto radiansToRotateBy = dt * 0.001f;
		float4x4 rotMat;
		rotMat.rotate(float3{ 0, radiansToRotateBy, 0 });
		float4 eyePos(m_eyePos.x, m_eyePos.y, m_eyePos.z, 1.0f);
		eyePos = eyePos * rotMat;
		m_eyePos = { eyePos.x, eyePos.y, eyePos.z };

		const auto view = ToFloat4x4(Matrix4f::LookAtLHMatrix(m_eyePos, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));
		const auto proj = ToFloat4x4(Matrix4f::PerspectiveFovLHMatrix(0.25f * f_PI, AspectRatio(), 1.0f, 125.0f));
		const float4x4 viewProj =   view * proj;
		
		*m_cb0 = SceneConstantBuffer{
			.projectionToWorld = viewProj.inverse(),
			.cameraPosition = eyePos,
			.lightPosition = {0.0f, 1.8f, -3.0f, 0.0f},
			.lightAmbientColor = {0.5f, 0.5f, 0.5f, 1.0f},
			.lightDiffuseColor = {0.5f, 0.0f, 0.0f, 1.0f}
		};
	}

	void DrawScene() override
	{
		FrameGraph fg;
		m_pDevice->BeginDrawFrameGraph(&fg);
		fg.DrawRenderPass(m_rtPass.get());
		m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
		m_pDevice->EndDrawFrameGraph();
	}

	std::unique_ptr<RenderPass> m_rtPass;
	shared_ptr<ConstantBuffer<SceneConstantBuffer>> m_cb0;
	shared_ptr<ConstantBuffer<CubeConstantBuffer>> m_cb1;
	shared_ptr<Texture2D> m_uavTex;

	Vector3f m_eyePos = { 0.0f, 2.0f, -5.0f };

	SceneData m_scene;
};

FORWARD_APPLICATION_MAIN(RaytracingSimpleLighting, 1920, 1080);