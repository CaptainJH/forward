#include "Application.h"
#include "effects/AutodeskStandardSurface.h"

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
	shared_ptr<forward::AutodeskStandardSurface> m_standardSurface;
};

void ModelViewer::UpdateScene(f32 dt)
{
	m_standardSurface->Update(dt);
}

void ModelViewer::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_standardSurface->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool ModelViewer::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	auto sceneData = SceneData::LoadFromFileForStandSurface(L"shaderball.glb", m_pDevice->mLoadedResourceMgr);
	m_standardSurface = make_shared<AutodeskStandardSurface>(sceneData);
	m_standardSurface->envRadianceTex = make_shared<Texture2D>("envRadianceTex", L"Lights/san_giuseppe_bridge_split.hdr");
	m_standardSurface->envIrradianceTex = make_shared<Texture2D>("envIrradianceTex", L"Lights/irradiance/san_giuseppe_bridge_split.hdr");
	m_standardSurface->SetupRenderPass(*m_pDevice);

	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	auto viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
	auto projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);
	auto viewProjMat = viewMat * projMat;
	auto worldMat = Matrix4f::Identity();
	auto worldInvT = worldMat.Inverse().Transpose();
	m_standardSurface->mUpdateFunc = [=](f32 /*dt*/) {
		*m_standardSurface->mCB0 = {
			.worldMat = worldMat,
			.viewProjMat = viewProjMat,
			.worldInverseTransMat = worldInvT
		};
		auto cb1 = CB1();
		cb1.u_viewPosition = pos;
		*m_standardSurface->mCB1 = cb1;
		*m_standardSurface->mCB2 = CB2();
	};

	return true;
}

FORWARD_APPLICATION_MAIN(ModelViewer, 1920, 1080);
