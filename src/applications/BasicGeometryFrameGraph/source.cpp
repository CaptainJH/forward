#include "Application.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"

#include "SceneData.h"

using namespace forward;

class BasicGeometryFrameGraph : public Application
{
public:
	BasicGeometryFrameGraph(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"BasicGeometryFrameGraph";
#ifdef WINDOWS
		RenderType = RendererType::Renderer_Forward_DX12;
#endif
	}

	~BasicGeometryFrameGraph()
	{
	}

    bool Init() override;
    void OnResize() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	//void OnSpace() override;

private:

	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	shared_ptr<ConstantBuffer<Matrix4f>> m_constantBuffer;

	std::unique_ptr<RenderPass> m_renderPass;
	SceneData m_scene;
};

void BasicGeometryFrameGraph::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	auto worldMat = Matrix4f::RotationMatrixY(frames);// *Matrix4f::RotationMatrixX(frames);
	*m_constantBuffer = worldMat * m_viewMat * m_projMat;
}

void BasicGeometryFrameGraph::DrawScene()
{
	FrameGraph fg;
	m_pRender2->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass.get());
	m_pRender2->DrawScreenText("Hello FrameGraph!", 10, 50, Colors::Red);
	m_pRender2->EndDrawFrameGraph();
}

bool BasicGeometryFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_scene = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pRender2->mLoadedResourceMgr);

	m_renderPass = std::make_unique<RenderPass>(
	[&](RenderPassBuilder& builder, PipelineStateObject& pso) {

		// Build the view matrix.
		Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
		Vector3f target; target.MakeZero();
		Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
		m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
		// Build the projection matrix
		m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);

		// setup shaders
		pso.m_VSState.m_shader = forward::make_shared<VertexShader>("HelloFrameGraphVS", L"BasicShader", L"VSMain_P_UV");
		//pso.m_VSState.m_shader = forward::make_shared<VertexShader>("HelloFrameGraphVS", L"BasicShader", L"VSMain");
		pso.m_PSState.m_shader = forward::make_shared<PixelShader>("HelloFrameGraphPS", L"BasicShader", L"PSMain");

        //pso.m_PSState.m_shaderResources[0] = make_shared<Texture2D>("DDS_Tex", L"bricks.dds");
		pso.m_PSState.m_shaderResources[0] = make_shared<Texture2D>("helmet_albedo", L"Default_albedo.jpg");
//        pso.m_PSState.m_shaderResources[1] = make_shared<TextureCube>("DDS_Cube", L"snowcube1024.dds");
        pso.m_PSState.m_samplers[0] = make_shared<SamplerState>("TexSamp");

		// setup geometry
		//m_geometry = std::make_unique<SimpleGeometry>("Sphere", forward::GeometryBuilder<forward::GP_SPHERE>(1.0f, 15, 20));
		builder << m_scene.mMeshData[0];

		// setup constant buffer
		m_constantBuffer = make_shared<ConstantBuffer<Matrix4f>>("CB");
		pso.m_VSState.m_constantBuffers[0] = m_constantBuffer;

		// setup render states
		pso.m_OMState.m_renderTargetResources[0] = m_pRender2->GetDefaultRT();
		pso.m_OMState.m_depthStencilResource = m_pRender2->GetDefaultDS();
	},
	[&](Renderer& render) {
		render.DrawIndexed(m_scene.mMeshData[0].GetIndexCount());
	});


	return true;
}

void BasicGeometryFrameGraph::OnResize()
{
	Application::OnResize();
}

//void BasicGeometryFrameGraph::OnSpace()
//{
//    mAppPaused = !mAppPaused;
//}


FORWARD_APPLICATION_MAIN(BasicGeometryFrameGraph, 1920, 1080);
