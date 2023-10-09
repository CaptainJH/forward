#include "Application.h"
#include "FrameGraph/Geometry.h"
#include "renderers/ReferencePTRenderer.h"

using namespace forward;

class ReferencePT : public Application
{
public:
	ReferencePT(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Reference Path Tracer";
	}

	~ReferencePT()
	{
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!Application::Init())
			return false;

		mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
		mFPCamera.SetPosition(float3(0.0f, 0.0f, -3.0f));
		m_scene = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);

		m_pt = make_shared<ReferencePTRenderer>(m_scene);
		m_pt->SetupRenderPass(*m_pDevice);

		m_pt->mUpdateFunc = [&](f32) {
			auto viewMat = mFPCamera.GetViewMatrix();
			static float4x4 lastViewMat = viewMat;
			static u32 accumulatedFrames = 0;
			bool resetAccumulation = !lastViewMat.equalWithAbsError(viewMat, std::numeric_limits<f32>::epsilon());
			if (resetAccumulation)
				accumulatedFrames = 1;
			else
				++accumulatedFrames;

			*m_pt->m_cb = RaytracingData{
				.view = viewMat.inverse(),
				.proj = mFPCamera.GetProjectionMatrix(),

				.skyIntensity = 3.0f,
				.lightCount = 0,
				.frameNumber = ++m_frames,
				.maxBounces = 8,

				.exposureAdjustment = 0.2f,
				.accumulatedFrames = accumulatedFrames,
				.enableAntiAliasing = TRUE,
				.focusDistance = 10.0f,

				.apertureSize = 0.0f,
				.enableAccumulation = resetAccumulation ? FALSE : TRUE,

				.lights = {}
			};

			lastViewMat = viewMat;
			};

		return true;
	}

protected:
	void UpdateScene(f32 dt) override
	{
		mFPCamera.UpdateViewMatrix();
		m_pt->Update(dt);
	}

	u32 m_frames = 0U;

	void DrawScene() override
	{
		FrameGraph fg;
		m_pDevice->BeginDrawFrameGraph(&fg);
		m_pt->DrawEffect(&fg);
		m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
		m_pDevice->EndDrawFrameGraph();
	}

	shared_ptr<ReferencePTRenderer> m_pt;
	SceneData m_scene;
};

FORWARD_APPLICATION_MAIN(ReferencePT, 1920, 1080);