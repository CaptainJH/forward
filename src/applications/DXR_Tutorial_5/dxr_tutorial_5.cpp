#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"

using namespace forward;

class DXR_Tutorial_5 : public Application
{
public:
	DXR_Tutorial_5(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 5";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:
	shared_ptr<RasterGBufferRenderer> m_gBufferRender;
	u32 m_frames = 0U;
};

void DXR_Tutorial_5::UpdateScene(f32 dt)
{
	mFPCamera.UpdateViewMatrix();
	m_gBufferRender->Update(dt);
}

void DXR_Tutorial_5::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_gBufferRender->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_5::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"D:/Documents/GitHub/Mastering-Graphics-Programming-with-Vulkan/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf", m_pDevice->mLoadedResourceMgr);
	m_gBufferRender = make_shared<RasterGBufferRenderer>(sceneData, mClientWidth, mClientHeight);
	m_gBufferRender->SetupRenderPass(*m_pDevice);

	m_gBufferRender->mUpdateFunc = [=](f32) {
		m_gBufferRender->updateConstantBuffer(mFPCamera.GetViewMatrix(), mFPCamera.GetProjectionMatrix());
		*m_gBufferRender->mCB1 = mFPCamera.GetPosition();
		};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_5, 1920, 1080);
