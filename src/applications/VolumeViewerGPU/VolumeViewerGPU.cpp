#pragma warning( push )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#include <OpenVDB/openvdb.h>
#pragma warning( pop )

#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"

using namespace forward;

class VolumeViewerGPU : public Application
{
	struct CB
	{
		float4x4 ProjectionToWorld;
		float4   CameraPosition;
		u32		FrameCount;
	};

public:
	VolumeViewerGPU(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Volume Rendering demo on GPU";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() { m_pause = !m_pause; }

private:
	shared_ptr<Texture2D> m_uavRT;
	shared_ptr<ConstantBuffer<CB>> m_cb;
	std::unique_ptr<RenderPass> m_volumePass;
	u32 m_frames = 0U;
	bool m_pause = false;
};

void VolumeViewerGPU::UpdateScene(f32)
{
	mFPCamera.UpdateViewMatrix();
	*m_cb = {
		.ProjectionToWorld = (mFPCamera.GetViewMatrix() * mFPCamera.GetProjectionMatrix()).inverse(),
		.CameraPosition = mFPCamera.GetPosition(),
		.FrameCount = m_frames++,
	};
}

void VolumeViewerGPU::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_volumePass.get());
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool VolumeViewerGPU::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	float3 camPos = { 0, 20.0f, -50.0f };
	mFPCamera.SetPosition(camPos);
	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 10000.0f);
	mFPCamera.LookAt(camPos, float3(0.0f), float3(0, 1, 0));

	m_volumePass = std::make_unique<RenderPass>([&](RenderPassBuilder& /*builder*/, ComputePipelineStateObject& pso) {

		m_cb = make_shared<ConstantBuffer<CB>>("Volume_CB");
		pso.m_CSState.m_constantBuffers[0] = m_cb;

		auto rt = m_pDevice->GetDefaultRT();
		m_uavRT = make_shared<Texture2D>("Volume_UAV_OUTPUT", DF_R8G8B8A8_UNORM, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
		m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		pso.m_CSState.m_uavShaderRes[0] = m_uavRT;

		// setup shaders
		pso.m_CSState.m_shader = make_shared<ComputeShader>("Volume_Shader", L"VolumeRendering", "VolumeMain");
		},
		[&](CommandList& cmdList) {
			constexpr u32 x = 640 / 8;
			constexpr u32 y = 480 / 8;
			cmdList.Dispatch(x, y, 1);
			cmdList.CopyResource(*m_pDevice->GetCurrentSwapChainRT(), *m_uavRT);
		});

	return true;
}

FORWARD_APPLICATION_MAIN(VolumeViewerGPU, 640, 480);
