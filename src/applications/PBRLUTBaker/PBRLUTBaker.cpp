#include "ApplicationWin.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"
#include <iostream>

using namespace forward;

class PBRLUTBaker : public Application
{
public:
	PBRLUTBaker(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"PBRLUTBaker";
		DeviceType = DeviceType::Device_Forward_DX12;
		mAppType = AT_OffScreen;
	}

	~PBRLUTBaker()
	{
	}

	bool Init() override;
	void SaveRT();

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	std::unique_ptr<RenderPass> m_renderPass;
	forward::shared_ptr<Texture2D> m_uavTex;
};

i32 main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	PBRLUTBaker theApp(1200, 800);

	if (!theApp.Init())
		return 0;

	theApp.Run();
	theApp.SaveRT();
}

void PBRLUTBaker::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void PBRLUTBaker::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass.get());
	m_pDevice->EndDrawFrameGraph();
}

bool PBRLUTBaker::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_uavTex = forward::make_shared<Texture2D>("UAV_Tex", forward::DF_B8G8R8A8_UNORM, 1024, 1024, forward::TextureBindPosition::TBP_Shader);
	m_renderPass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& /*builder*/, PipelineStateObject& pso) {
		// setup shaders
		pso.m_CSState.m_shader = forward::make_shared<ComputeShader>("PBR_Baker", L"PBRShader", L"BakerMain");
		pso.m_CSState.m_uavShaderRes[0] = m_uavTex;

		//// setup geometry
		//auto geometry = std::make_unique<SimpleGeometry>("Geometry", forward::GeometryBuilder<forward::GP_SCREEN_QUAD>());
		//builder << *geometry;

		//// setup render targets
		//auto rtPtr = forward::make_shared<Texture2D>(std::string("RT"), DF_R8G8B8A8_UNORM,
		//	mClientWidth, mClientHeight, TextureBindPosition::TBP_RT);
		//rtPtr->SetUsage(ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL);
		//pso.m_OMState.m_renderTargetResources[0] = rtPtr;

		//auto dsPtr = forward::make_shared<Texture2D>(std::string("DS"), DF_D32_FLOAT,
		//	mClientWidth, mClientHeight, TextureBindPosition::TBP_DS);
		//pso.m_OMState.m_depthStencilResource = dsPtr;
	},
		[](Device& device) {
		device.Draw(4);
	});


	return true;
}

void PBRLUTBaker::SaveRT()
{
	m_pDevice->SaveRenderTarget(L"OffScreenRenderingResultDX12.bmp", m_renderPass->GetPSO());
}