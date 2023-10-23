#include "Application.h"
#include "renderers/SimpleAlbedo.h"

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
	shared_ptr<forward::SimpleAlbedoRenderer> m_albedoEffect;
};

void DXR_Tutorial_3::UpdateScene(f32 dt)
{
	mFPCamera.UpdateViewMatrix();
	m_albedoEffect->Update(dt);
}

void DXR_Tutorial_3::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_albedoEffect->DrawEffect(&fg);
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
	m_albedoEffect = make_shared<SimpleAlbedoRenderer>(sceneData);
	m_albedoEffect->SetupRenderPass(*m_pDevice);

	m_albedoEffect->mUpdateFunc = [=](f32) {
		m_albedoEffect->updateConstantBuffer(mFPCamera.GetViewMatrix(), mFPCamera.GetProjectionMatrix());
	};

	return true;
}

FORWARD_APPLICATION_MAIN(DXR_Tutorial_3, 1920, 1080);
