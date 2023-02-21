#include "Application.h"
#include "effects/AutodeskStandardSurface.h"
#include "ArcBall.h"

using namespace forward;

class ModelViewer : public Application
{
public:
	ModelViewer(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"ModelViewer";
		m_arcCam.SetWindow(width, height);
		m_arcCam.SetRadius(2.0f);
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM /*btnState*/, i32 /*x*/, i32 /*y*/) override;
	void OnMouseUp(WPARAM /*btnState*/, i32 /*x*/, i32 /*y*/) override;
	void OnMouseMove(WPARAM /*btnState*/, i32 /*x*/, i32 /*y*/) override;

	Vector3f rotateVector(Quaternion<f32> q, Vector3f v);

private:
	shared_ptr<forward::AutodeskStandardSurface> m_standardSurface;
	ArcBall m_arcCam;
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

	m_standardSurface->mUpdateFunc = [&](f32 /*dt*/) {

		auto camRot = m_arcCam.GetQuat();
		Vector3f target = Vector3f(0.0f, 1.0f, 0.0f);
		Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
		Vector3f dir = rotateVector(camRot, Vector3f(0.0f, 0.0f, -1.0f));
		auto camPos = target + dir * 2.0f;
		Vector3f newUp = rotateVector(camRot, up);
	
		auto viewMat = Matrix4f::LookAtLHMatrix(camPos, target, newUp);
		auto projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);
		auto viewProjMat = viewMat * projMat;
		auto worldMat = Matrix4f::Identity();
		auto worldInvT = worldMat.Inverse().Transpose();

		*m_standardSurface->mCB0 = {
			.worldMat = worldMat,
			.viewProjMat = viewProjMat,
			.worldInverseTransMat = worldInvT
		};
		auto cb1 = CB1();
		cb1.u_viewPosition = camPos;
		*m_standardSurface->mCB1 = cb1;
		*m_standardSurface->mCB2 = CB2();
	};

	return true;
}

void ModelViewer::OnMouseDown(WPARAM btnState, i32 x, i32 y)
{
	if (btnState == MK_LBUTTON)
	{
		m_arcCam.OnBegin(x, y);
	}
}

void ModelViewer::OnMouseUp(WPARAM /*btnState*/, i32 /*x*/, i32 /*y*/)
{
	m_arcCam.OnEnd();
}

void ModelViewer::OnMouseMove(WPARAM /*btnState*/, i32 x, i32 y)
{
	if (m_arcCam.IsDragging())
	{
		// Rotate camera
		m_arcCam.OnMove(x, y);
	}
}

Vector3f ModelViewer::rotateVector(Quaternion<f32> q, Vector3f v)
{
	Quaternion<f32> vec(0.0f, v.x, v.y, v.z);
	Quaternion<f32> inv(q.w, -q.x, -q.y, -q.z);
	auto result = q * vec * inv;
	return Vector3f(result.x, result.y, result.z);
}

FORWARD_APPLICATION_MAIN(ModelViewer, 1920, 1080);
