#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"

using namespace forward;

class DXR_Tutorial_3 : public Application
{
public:
	DXR_Tutorial_3(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"DXR Tutorial 3";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:
	shared_ptr<forward::RasterGBufferRenderer> m_gBufferRender;
};

void DXR_Tutorial_3::UpdateScene(f32 dt)
{
	mFPCamera.UpdateViewMatrix();
	m_gBufferRender->Update(dt);
}

void DXR_Tutorial_3::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_gBufferRender->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool DXR_Tutorial_3::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"D:/Documents/GitHub/Mastering-Graphics-Programming-with-Vulkan/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf", m_pDevice->mLoadedResourceMgr);
	m_gBufferRender = make_shared<RasterGBufferRenderer>(sceneData);
	m_gBufferRender->m_rt_color = make_shared<Texture2D>("RT_Color", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_RT);
	m_gBufferRender->m_depth = make_shared<Texture2D>("RT_Depth", DF_D24_UNORM_S8_UINT, mClientWidth, mClientHeight, TextureBindPosition::TBP_DS);
	m_gBufferRender->m_gBuffer_Pos = make_shared<Texture2D>("RT_gBuffer_Pos", DF_R32G32B32A32_FLOAT, mClientWidth, mClientHeight, TextureBindPosition::TBP_RT);
	m_gBufferRender->m_gBuffer_Normal = make_shared<Texture2D>("RT_gBuffer_Normal", DF_R32G32B32A32_FLOAT, mClientWidth, mClientHeight, TextureBindPosition::TBP_RT);
	m_gBufferRender->SetupRenderPass(*m_pDevice);

	m_gBufferRender->mUpdateFunc = [=](f32) {
		m_gBufferRender->updateConstantBuffer(mFPCamera.GetViewMatrix(), mFPCamera.GetProjectionMatrix());
	};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_3, 1920, 1080);
