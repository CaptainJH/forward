#include "utilities/Application.h"
#include "renderers/RasterGBufferRenderer.h"
#include "utilities/Log.h"

using namespace forward;

class SkyRenderingGPU : public Application
{
public:
	SkyRenderingGPU(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Sky Rendering demo on GPU";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() { m_pause = !m_pause; }

private:
	shared_ptr<Texture2D> m_uavRT;
	shared_ptr<ConstantBuffer<f32>> m_cb;
	std::unique_ptr<RenderPass> m_skyPass;
	u32 m_frames = 0U;
	bool m_pause = false;
};

void SkyRenderingGPU::UpdateScene(f32 dt)
{
	static f32 angle = 0.0f;
	if (!m_pause)
		angle += dt * 0.001f;
	*m_cb = angle;
}

void SkyRenderingGPU::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_skyPass.get());
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool SkyRenderingGPU::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_skyPass = std::make_unique<RenderPass>([&](RenderPassBuilder& /*builder*/, ComputePipelineStateObject& pso) {

		m_cb = make_shared<ConstantBuffer<f32>>("SKY_CB");
		pso.m_CSState.m_constantBuffers[0] = m_cb;

		auto rt = m_pDevice->GetDefaultRT();
		m_uavRT = make_shared<Texture2D>("SKY_UAV_OUTPUT", DF_R8G8B8A8_UNORM, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
		m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		pso.m_CSState.m_uavShaderRes[0] = m_uavRT;

		// setup shaders
		pso.m_CSState.m_shader = make_shared<ComputeShader>("SKY_Shader", L"SkyRendering", "SkyMain");
		},
		[&](CommandList& cmdList) {
			constexpr u32 x = 1024 / 8;
			constexpr u32 y = 1024 / 8;
			cmdList.Dispatch(x, y, 1);
			cmdList.CopyResource(*m_pDevice->GetCurrentSwapChainRT(), *m_uavRT);
		});

	return true;
}

FORWARD_APPLICATION_MAIN(SkyRenderingGPU, 1024, 1024);
