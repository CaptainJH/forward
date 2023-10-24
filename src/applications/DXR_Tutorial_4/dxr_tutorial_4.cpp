#include "Application.h"
#include "renderers/RTGBufferRenderer.h"

using namespace forward;

class DXR_Tutorial_4 : public Application
{
public:
	DXR_Tutorial_4(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 4";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:
	shared_ptr<RTGBufferRenderer> m_gBufferRender;
	u32 m_frames = 0U;
};

void DXR_Tutorial_4::UpdateScene(f32 dt)
{
	mFPCamera.UpdateViewMatrix();
	m_gBufferRender->Update(dt);
}

void DXR_Tutorial_4::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_gBufferRender->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_4::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"D:/Documents/GitHub/Mastering-Graphics-Programming-with-Vulkan/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf", m_pDevice->mLoadedResourceMgr);
	m_gBufferRender = make_shared<RTGBufferRenderer>(sceneData);
	m_gBufferRender->SetupRenderPass(*m_pDevice);

	m_gBufferRender->mUpdateFunc = [&](f32) {
		auto viewMat = mFPCamera.GetViewMatrix();
		static float4x4 lastViewMat = viewMat;
		static u32 accumulatedFrames = 0;
		bool resetAccumulation = !lastViewMat.equalWithAbsError(viewMat, std::numeric_limits<f32>::epsilon());
		if (resetAccumulation)
			accumulatedFrames = 1;
		else
			++accumulatedFrames;

		*m_gBufferRender->m_cb = RaytracingData{
			.view = viewMat.inverse(),
			.proj = mFPCamera.GetProjectionMatrix(),

			.skyIntensity = 3.0f,
			.lightCount = 0,
			.frameNumber = ++m_frames,
			.maxBounces = 1,

			.exposureAdjustment = 0.2f,
			.accumulatedFrames = 1,
			.enableAntiAliasing = TRUE,
			.focusDistance = 10.0f,

			.apertureSize = 0.0f,
			.enableAccumulation = FALSE,

			.lights = {}
		};

		lastViewMat = viewMat;
		};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_4, 1920, 1080);
