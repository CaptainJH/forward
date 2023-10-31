#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"
#include "renderers/RT_Temp.h"

using namespace forward;

class DXR_Tutorial_4_2 : public Application
{
public:
	DXR_Tutorial_4_2(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 4_2: Temp";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override { m_useGI = !m_useGI; }

private:
	shared_ptr<RTTemp> m_tempRender;
	shared_ptr<RasterGBufferRenderer> m_rasterGBufferRender;
	u32 m_frames = 0U;
	bool m_useGI = false;
	float3 m_lightPos = float3(1.12f, 9.0f, 0.6f);
};

void DXR_Tutorial_4_2::UpdateScene(f32 dt)
{
	mFPCamera.UpdateViewMatrix();
	m_rasterGBufferRender->Update(dt);
	m_tempRender->Update(dt);
}

void DXR_Tutorial_4_2::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_rasterGBufferRender->DrawEffect(&fg);
	m_tempRender->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_4_2::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"Sponza/glTF/Sponza.gltf", m_pDevice->mLoadedResourceMgr);
	m_rasterGBufferRender = make_shared<RasterGBufferRenderer>(sceneData, mClientWidth, mClientHeight);
	m_rasterGBufferRender->SetupRenderPass(*m_pDevice);

	m_rasterGBufferRender->mUpdateFunc = [=](f32) {
		m_rasterGBufferRender->updateConstantBuffer(mFPCamera.GetViewMatrix(), mFPCamera.GetProjectionMatrix());
		*m_rasterGBufferRender->mCB1 = mFPCamera.GetPosition();
		};

	m_tempRender = make_shared<RTTemp>(sceneData);
	m_tempRender->SetupRenderPass(*m_pDevice);

	m_tempRender->mUpdateFunc = [&](f32) {
		auto viewMat = mFPCamera.GetViewMatrix();
		static float4x4 lastViewMat = viewMat;
		static u32 accumulatedFrames = 0;
		bool resetAccumulation = !lastViewMat.equalWithAbsError(viewMat, std::numeric_limits<f32>::epsilon());
		if (resetAccumulation)
			accumulatedFrames = 1;
		else
			++accumulatedFrames;

		*m_tempRender->m_cb = RaytracingData{
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

		*m_tempRender->m_gi_cb = {
			.g_LightPos = m_lightPos,
			.g_use_GI = m_useGI ? 1U : 0U,
		};

		lastViewMat = viewMat;
		};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_4_2, 1920, 1080);
