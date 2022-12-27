#include "Application.h"
#include "effects/SimpleAlbedo.h"

using namespace forward;

class ModelViewer : public Application
{
public:
	ModelViewer(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"ModelViewer";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:
	shared_ptr<forward::SimpleAlbedo> m_albedoEffect;
};

void ModelViewer::UpdateScene(f32 dt)
{
	m_albedoEffect->Update(dt);
}

void ModelViewer::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_albedoEffect->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool ModelViewer::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	auto sceneData = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);
	m_albedoEffect = make_shared<SimpleAlbedo>(sceneData);
	m_albedoEffect->mAlbedoTex = make_shared<Texture2D>("helmet_albedo", L"DamagedHelmet/Default_albedo.jpg");
	m_albedoEffect->SetupRenderPass(*m_pDevice);

	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	auto viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
	auto projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);
	m_albedoEffect->mUpdateFunc = [=](f32 dt) {
		static f32 frames = 0.0f;
		frames += dt * 0.001f;
		*m_albedoEffect->mCB = Matrix4f::RotationMatrixY(frames) * viewMat * projMat;
	};

	return true;
}

FORWARD_APPLICATION_MAIN(ModelViewer, 1920, 1080);
