#include "utilities/Application.h"
#include "renderers/RTGBufferRenderer.h"
#include "renderers/RTAORenderer.h"
#include "utilities/Log.h"
#include <random>

using namespace forward;

class DXR_Tutorial_8 : public Application
{
public:
	DXR_Tutorial_8(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 8: Depth-of-field using a simple thin-lens camera";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:
	shared_ptr<RTGBufferRenderer> m_gBufferRender;
	shared_ptr<RTAORenderer> m_rtaoRenderer;
	u32 m_frames = 0U;
	u32 m_accumulatedFrames = 0U;
};

void DXR_Tutorial_8::UpdateScene(f32 dt)
{
	m_accumulatedFrames = mFPCamera.UpdateViewMatrix() ? 0U : m_accumulatedFrames + 1;
	m_gBufferRender->Update(dt);
	m_rtaoRenderer->Update(dt);
}

void DXR_Tutorial_8::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_gBufferRender->DrawEffect(&fg);
	m_rtaoRenderer->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_8::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"Sponza/glTF/Sponza.gltf", m_pDevice->mLoadedResourceMgr);
	m_gBufferRender = make_shared<RTGBufferRenderer>(sceneData);
	m_gBufferRender->SetupRenderPass(*m_pDevice);

	m_gBufferRender->mUpdateFunc = [=](f32) {
		*m_gBufferRender->m_cb = RaytracingData{
			.view = mFPCamera.GetViewMatrix().inverse(),
			.proj = mFPCamera.GetProjectionMatrix(),

			.skyIntensity = 3.0f,
			.lightCount = 0,
			.frameNumber = ++m_frames,
			.maxBounces = 1,

			.exposureAdjustment = 0.2f,
			.accumulatedFrames = 1,
			.enableAntiAliasing = TRUE,
			.focusDistance = 10.0f,

			.apertureSize = 0.2f,
			.enableAccumulation = FALSE,

			.lights = {}
		};
		};

	m_rtaoRenderer = make_shared<RTAORenderer>(sceneData);
	m_rtaoRenderer->m_posWorld = m_gBufferRender->m_gBuffer_Pos;
	m_rtaoRenderer->m_normalWorld = m_gBufferRender->m_gBuffer_Normal;
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

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_8, 1920, 1080);
