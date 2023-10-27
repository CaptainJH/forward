#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"
#include "renderers/RTLambertRenderer.h"
#include <random>

using namespace forward;

class DXR_Tutorial_9 : public Application
{
public:
	DXR_Tutorial_9(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 9: A simple Lambertian material model with ray traced shadows";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override { m_applyJitter = !m_applyJitter; }
	void OnChar(i8 key, bool pressed) override
	{
		Application::OnChar(key, pressed);
		if (key == 'L')
			m_lightPos.x -= 1.0f;
		else if (key == 'R')
			m_lightPos.x += 1.0f;
		else if (key == 'U')
			m_lightPos.y += 1.0f;
		else if (key == 'J')
			m_lightPos.y -= 1.0f;
		else if (key == 'F')
			m_lightPos.z += 1.0f;
		else if (key == 'N')
			m_lightPos.z -= 1.0f;
	}

private:
	shared_ptr<RasterGBufferRenderer> m_gBufferRender;
	shared_ptr<RTLambertRenderer> m_lambertRenderer;
	u32 m_frames = 0U;
	u32 m_accumulatedFrames = 0U;

	// for camera jittering
	// camera jittering is the content for dxr_tutorial_7, 
	// and I implement it here, so I'll skip dxr_tutorial_7
	std::uniform_real_distribution<f32> m_rngDist;
	std::mt19937 m_rng;
	bool m_applyJitter = true;

	float3 m_lightPos = float3(1.12f, 9.0f, 0.6f);
};

void DXR_Tutorial_9::UpdateScene(f32 dt)
{
	m_accumulatedFrames = mFPCamera.UpdateViewMatrix() ? 0U : m_accumulatedFrames + 1;
	m_gBufferRender->Update(dt);
	m_lambertRenderer->Update(dt);
}

void DXR_Tutorial_9::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_gBufferRender->DrawEffect(&fg);
	m_lambertRenderer->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_9::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	auto now = std::chrono::high_resolution_clock::now();
	auto msTime = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	m_rng = std::mt19937(u32(msTime.time_since_epoch().count()));

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"Sponza/glTF/Sponza.gltf", m_pDevice->mLoadedResourceMgr);
	m_gBufferRender = make_shared<RasterGBufferRenderer>(sceneData, mClientWidth, mClientHeight);
	m_gBufferRender->SetupRenderPass(*m_pDevice);

	m_gBufferRender->mUpdateFunc = [=](f32) {
		float4x4 jitteringMat;
		if (m_applyJitter)
		{
			auto xJitter = (m_rngDist(m_rng) - 0.5f) / static_cast<f32>(mClientWidth);
			auto yJitter = (m_rngDist(m_rng) - 0.5f) / static_cast<f32>(mClientHeight);
			jitteringMat.translate(float3(xJitter, yJitter, 0.0f));
		}
		m_gBufferRender->updateConstantBuffer(mFPCamera.GetViewMatrix(), mFPCamera.GetProjectionMatrix() * jitteringMat);
		*m_gBufferRender->mCB1 = mFPCamera.GetPosition();
		};

	m_lambertRenderer = make_shared<RTLambertRenderer>(sceneData);
	m_lambertRenderer->m_posWorld = m_gBufferRender->m_gBuffer_Pos;
	m_lambertRenderer->m_normalWorld = m_gBufferRender->m_gBuffer_Normal;
	m_lambertRenderer->m_diffuse = m_gBufferRender->m_rt_color;
	m_lambertRenderer->SetupRenderPass(*m_pDevice);

	m_lambertRenderer->mUpdateFunc = [&](f32) {
		*m_lambertRenderer->m_cb = {
			.gLightIntensity = float3(1.0f, 1.0f, 1.0f),
			.gLightPos = m_lightPos,
			.gMinT = 1.0f,
		};
		if (m_lambertRenderer->m_cb_accumulation)
			*m_lambertRenderer->m_cb_accumulation = m_accumulatedFrames;
		};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_9, 1920, 1080);
