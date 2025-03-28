#include "utilities/Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"
#include <iostream>
#include "utilities/ProfilingHelper.h"
#include "utilities/Log.h"

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
	void UpdateScene(f32) override {}
	void DrawScene() override;

private:
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

	ProfilingHelper::BeginPixCapture("PBRLUTBaker_PIX_Capture.wpix");
	theApp.Run();
	ProfilingHelper::EndPixCapture();
	theApp.SaveRT();
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

	m_uavTex = forward::make_shared<Texture2D>("UAV_Tex", forward::DF_R16G16_FLOAT, 256, 256, forward::TextureBindPosition::TBP_Shader);
	m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
	m_renderPass = std::make_unique<RenderPass>(
		[&]([[maybe_unused]]RenderPassBuilder& builder, ComputePipelineStateObject& pso) {
			// setup shaders
			pso.m_CSState.m_shader = forward::make_shared<ComputeShader>("PBR_Baker", L"PBRShader", "BakerMain");
			builder.GetRenderPass()->m_cs.m_uavShaderRes[0] = m_uavTex;
		},
		[](CommandList& cmdList, RenderPass&) {
			cmdList.Dispatch(32, 32, 1);
		});

	return true;
}

void PBRLUTBaker::SaveRT()
{
	m_pDevice->SaveTexture(L"ComputeShaderResultDX12.dds", m_uavTex.get());
}