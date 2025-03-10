#include "utilities/Application.h"
#include "renderers/SimpleAlbedo.h"
#include "utilities/Log.h"

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
	shared_ptr<forward::SimpleAlbedoRenderer> m_albedoEffect;
};

void BasicGeometryFrameGraph::UpdateScene(f32 dt)
{
	mFPCamera.UpdateViewMatrix();
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

	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
	mFPCamera.SetPosition(0.0f, 0.0f, -3.0f);
	auto sceneData = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);
	//auto sceneData = SceneData::LoadFromFile(L"bathroom/LAZIENKA.gltf", m_pDevice->mLoadedResourceMgr);
	std::vector<SceneData::Instance*> instancesWithBaseTex;
	std::for_each(sceneData.mInstances.begin(), sceneData.mInstances.end(), [&](auto& ins) {
		if (!sceneData.mMaterials[ins.materialId].baseColorTexName.empty())
			instancesWithBaseTex.push_back(&ins);
		});
	auto instance = instancesWithBaseTex.back();
	m_albedoEffect = make_shared<SimpleAlbedoRenderer>(sceneData);
	m_albedoEffect->SetupRenderPass(*m_pDevice);

	auto object2WorldMatrix = instance->mat;
	m_albedoEffect->mUpdateFunc = [=](f32 dt) {
		static f32 frames = 0.0f;
		frames += dt * 0.001f;
		float4x4 rotM;
		rotM.rotate(float3(0, frames, 0));
		*m_albedoEffect->mCBs[0] = {
			.ViewProjMatrix = mFPCamera.GetViewMatrix() * mFPCamera.GetProjectionMatrix(),
			.WorldMatrix = object2WorldMatrix * rotM,
		};
	};

	return true;
}

FORWARD_APPLICATION_MAIN(BasicGeometryFrameGraph, 1920, 1080);
