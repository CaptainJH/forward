#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"
#include "renderers/RTLambertRenderer.h"
#include "Log.h"
#include <imgui/imgui.h>

using namespace forward;

class DXR_Tutorial_12 : public Application
{
public:
	DXR_Tutorial_12(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 12: Extending our Lambertian shading to add one-bounce global illumination";
	}

	bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override { m_useGI = !m_useGI; m_resetAccumulation = true; }
	void OnGUI() override;

private:
	shared_ptr<RTLambertRenderer> m_lambertGIRender;
	shared_ptr<RasterGBufferRenderer> m_rasterGBufferRender;
	u32 m_frames = 0U;
	bool m_useGI = true;
	bool m_useLocal = true;
	float3 m_lightPos = float3(1.12f, 9.0f, 0.6f);
	u32 m_accumulatedFrames = 0U;
	bool m_resetAccumulation = false;
};

void DXR_Tutorial_12::UpdateScene(f32 dt)
{
	m_accumulatedFrames = (mFPCamera.UpdateViewMatrix() || m_resetAccumulation) ? 0U : m_accumulatedFrames + 1;
	if (m_accumulatedFrames == 0) m_resetAccumulation = false;
	m_rasterGBufferRender->Update(dt);
	m_lambertGIRender->Update(dt);
}

void DXR_Tutorial_12::OnGUI()
{
	Application::OnGUI();

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 55), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("DXR_Tutorial_12 Settings:", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	ImGui::LabelText("", "Light Position:");
	bool updated = false;
	updated |= ImGui::SliderFloat3("", &m_lightPos.x, 0.0f, 10.0f);
	updated |= ImGui::Checkbox("Use GI", &m_useGI);
	updated |= ImGui::Checkbox("Direct Lighting", &m_useLocal);

	if (updated) m_resetAccumulation = true;

	ImGui::End();
}

void DXR_Tutorial_12::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_rasterGBufferRender->DrawEffect(&fg);
	m_lambertGIRender->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_12::Init()
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

	m_lambertGIRender = make_shared<RTLambertRenderer>(sceneData);
	m_lambertGIRender->m_posWorld = m_rasterGBufferRender->m_gBuffer_Pos;
	m_lambertGIRender->m_normalWorld = m_rasterGBufferRender->m_gBuffer_Normal;
	m_lambertGIRender->m_diffuse = m_rasterGBufferRender->m_rt_color;
	m_lambertGIRender->SetupRenderPassWithGI(*m_pDevice);

	m_lambertGIRender->mUpdateFunc = [&](f32) {

		*m_lambertGIRender->m_rt_cb = RaytracingData{
			.view = mFPCamera.GetViewMatrix().inverse(),
			.proj = mFPCamera.GetProjectionMatrix(),

			.skyIntensity = 3.0f,
			.lightCount = 0,
			.frameNumber = ++m_frames,
			.maxBounces = 1,

			.exposureAdjustment = 0.2f,
			.accumulatedFrames = m_accumulatedFrames,
			.enableAntiAliasing = TRUE,
			.focusDistance = 10.0f,

			.apertureSize = 0.0f,
			.enableAccumulation = FALSE,

			.lights = {}
		};

		*m_lambertGIRender->m_gi_cb = {
			.g_LightPos = m_lightPos,
			.g_use_GI = m_useGI ? 1U : 0U,
			.g_use_Local = m_useLocal ? 1U : 0U,
		};

		*m_lambertGIRender->m_cb_accumulation = m_accumulatedFrames;

		};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_12, 1920, 1080);
