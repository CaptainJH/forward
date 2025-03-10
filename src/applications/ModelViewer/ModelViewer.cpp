#include "utilities/Application.h"
#include "renderers/AutodeskStandardSurface.h"
#include "renderers/MaterialXRenderer.h"
#include "utilities/ArcBall.h"
#include "../MaterialXReader/MaterialXReader.h"
#include "utilities/pystring.h"
#include "utilities/Log.h"
#include <filesystem>

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

	void OnMouseDown(u8 /*btnState*/, f32 /*x*/, f32 /*y*/) override;
	void OnMouseUp(u8 /*btnState*/, f32 /*x*/, f32 /*y*/) override;
	void OnMouseMove(u8 /*btnState*/, f32 /*x*/, f32 /*y*/) override;

	Vector3f rotateVector(Quaternion<f32> q, Vector3f v);

private:
	shared_ptr<forward::MaterialXRenderer> m_material;
	ArcBall m_arcCam;
	std::unordered_map<std::string, std::string> m_paramsPS;
};

void ModelViewer::UpdateScene(f32 dt)
{
	m_material->Update(dt);
}

void ModelViewer::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_material->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool ModelViewer::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	//std::filesystem::path materialXFilePath = "D:\\Documents\\GitHub\\MaterialX\\resources\\Materials\\Examples\\UsdPreviewSurface\\usd_preview_surface_glass.mtlx";
	std::filesystem::path materialXFilePath = "D:/Documents/GitHub/MaterialX_JHQ/MaterialX/resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
	std::string outVS, outPS;
	if (Forward_Read_MaterialX(materialXFilePath.string().c_str(), outVS, outPS, m_paramsPS) != 0)
	{
		assert(false);
		return false;
	}

	auto sceneData = SceneData::LoadFromFileForStandSurface(L"shaderball.glb", m_pDevice->mLoadedResourceMgr);
	m_material = make_shared<MaterialXRenderer>(sceneData, materialXFilePath.filename().replace_extension().string().c_str(), outVS.c_str(), outPS.c_str());
	m_material->envRadianceTex = make_shared<Texture2D>("u_envRadiance", L"Lights/san_giuseppe_bridge_split.hdr");
	m_material->envIrradianceTex = make_shared<Texture2D>("u_envIrradiance", L"Lights/irradiance/san_giuseppe_bridge_split.hdr");
	for (auto pair : m_paramsPS)
	{
		if (pair.second.starts_with("textures/"))
		{
			const auto v0 = pystring::split(pair.first, ".");
			assert(v0.size() == 2);
			std::filesystem::path texPath = materialXFilePath.parent_path() / pair.second;
			m_material->mTextures.emplace_back(make_shared<Texture2D>(v0[1].c_str(), texPath.wstring().c_str()));
		}
	}
	m_material->SetupRenderPass(*m_pDevice);

	m_material->mUpdateFunc = [&](f32 /*dt*/) {

		auto camRot = m_arcCam.GetQuat();
		Vector3f target = Vector3f(0.0f, 1.0f, 0.0f);
		Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
		Vector3f dir = rotateVector(camRot, Vector3f(0.0f, 0.0f, -1.0f));
		auto camPos = target + dir * 2.0f;
		Vector3f newUp = rotateVector(camRot, up);
	
		auto viewMat = Matrix4f::LookAtLHMatrix(camPos, target, newUp);
		auto projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * f_PI, AspectRatio(), 0.01f, 100.0f);
		auto viewProjMat = viewMat * projMat;
		auto worldMat = Matrix4f::Identity();
		auto worldInvT = worldMat.Inverse().Transpose();

		CB0 cb0 = {
			.worldMat = worldMat,
			.viewProjMat = viewProjMat,
			.worldInverseTransMat = worldInvT
		};
		CB1 cb1 = CB1();
		cb1.u_viewPosition = camPos;
		CB3 cb3;
		auto& lightData0 = cb3.u_lightData[0];
		lightData0.color = Vector3f(1.0f, 0.89447f, 0.56723f);
		lightData0.direction = Vector3f(0.51443f, -0.47901f, 0.71127f);
		lightData0.intensity = 2.52776f;
		lightData0.type = 1;

		for (auto& rp : m_material->m_renderPassVec)
		{
			auto& pso = rp.GetPSO<RasterPipelineStateObject>();
			if (!pso.m_VSState.m_constantBuffers[0])
				break;

			pso.m_VSState.SetConstantBufferDataFromPtr(0, &cb0);
			pso.m_PSState.SetConstantBufferDataFromPtr(1, &cb1);
			pso.m_PSState.SetConstantBufferDataFromStr(2, m_paramsPS);
			pso.m_PSState.SetConstantBufferDataFromPtr(3, &cb3);
		}
	};

	return true;
}

void ModelViewer::OnMouseDown(u8 btnState, f32 x, f32 y)
{
	if (btnState == 1)
	{
		m_arcCam.OnBegin(x, y);
	}
}

void ModelViewer::OnMouseUp(u8 /*btnState*/, f32 /*x*/, f32 /*y*/)
{
	m_arcCam.OnEnd();
}

void ModelViewer::OnMouseMove(u8 /*btnState*/, f32 x, f32 y)
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
