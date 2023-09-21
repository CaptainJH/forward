#include "Application.h"
#include "effects/SimpleAlbedo.h"

using namespace forward;

class BasicGeometryFrameGraph : public Application
{
public:
	BasicGeometryFrameGraph(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"BasicGeometryFrameGraph";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:
	shared_ptr<forward::SimpleAlbedo> m_albedoEffect;
};

void BasicGeometryFrameGraph::UpdateScene(f32 dt)
{
	m_albedoEffect->Update(dt);
}

void BasicGeometryFrameGraph::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	m_albedoEffect->DrawEffect(&fg);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

bool BasicGeometryFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	auto sceneData = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);
	//auto sceneData = SceneData::LoadFromFile(L"bathroom/LAZIENKA.gltf", m_pDevice->mLoadedResourceMgr);
	std::vector<SceneData::Instance*> instancesWithBaseTex;
	std::for_each(sceneData.mInstances.begin(), sceneData.mInstances.end(), [&](auto& ins) {
		if (!sceneData.mMaterials[ins.materialId].baseColorTexName.empty())
			instancesWithBaseTex.push_back(&ins);
		});
	auto instance = instancesWithBaseTex.back();
	m_albedoEffect = make_shared<SimpleAlbedo>(sceneData, instance->meshId);
	m_albedoEffect->mAlbedoTex = sceneData.mTextures[sceneData.mMaterials[instance->materialId].materialData.baseColorTexIdx];
	m_albedoEffect->SetupRenderPass(*m_pDevice);

	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target = { instance->translation.x, instance->translation.y, instance->translation.z };
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	const auto mat = instance->mat;
	const auto viewMat = ToFloat4x4(Matrix4f::LookAtLHMatrix(pos, target, up));
	const auto projMat = ToFloat4x4(Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f));
	m_albedoEffect->mUpdateFunc = [=](f32 dt) {
		static f32 frames = 0.0f;
		frames += dt * 0.001f;
		float4x4 rotM;
		rotM.rotate(float3(0, frames, 0));
		*m_albedoEffect->mCB = mat * rotM * viewMat * projMat;
	};

	return true;
}

FORWARD_APPLICATION_MAIN(BasicGeometryFrameGraph, 1920, 1080);
