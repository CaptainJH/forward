#include "ApplicationWin.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"
#include <iostream>
#include "ProfilingHelper.h"

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

	ProfilingHelper::BeginPixCapture("PBRLUTBaker_Capture2.wpix");
	theApp.Run();
	ProfilingHelper::EndPixCapture();
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

	m_uavTex = forward::make_shared<Texture2D>("UAV_Tex", forward::DF_R8G8B8A8_UNORM, 1024, 1024, forward::TextureBindPosition::TBP_Shader);
	m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
	m_renderPass = std::make_unique<RenderPass>(
		[&]([[maybe_unused]]RenderPassBuilder& builder, PipelineStateObject& pso) {
		// setup shaders
		pso.m_CSState.m_shader = forward::make_shared<ComputeShader>("PBR_Baker", L"PBRShader", L"BakerMain");
		pso.m_CSState.m_uavShaderRes[0] = m_uavTex;
		},
		[](Device& device) {
			device.GetCmdList().Dispatch(64, 64, 1);
	});


	return true;
}

void PBRLUTBaker::SaveRT()
{
	m_pDevice->SaveTexture(L"ComputeShaderResultDX12.dds", m_uavTex.get());
}