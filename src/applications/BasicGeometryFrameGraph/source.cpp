#include "ApplicationWin.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"

using namespace forward;

class BasicGeometryFrameGraph : public Application
{
public:
	BasicGeometryFrameGraph(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
	{
		mMainWndCaption = L"BasicGeometryFrameGraph";
		RenderType = RendererType::Renderer_Forward_DX12;
	}

	~BasicGeometryFrameGraph()
	{
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override;

private:

	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	shared_ptr<FrameGraphConstantBuffer<Matrix4f>> m_constantBuffer;

	std::unique_ptr<RenderPass> m_renderPass;
	std::unique_ptr<SimpleGeometry> m_geometry;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BasicGeometryFrameGraph theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void BasicGeometryFrameGraph::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	auto worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
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
		pso.m_VSState.m_shader = forward::make_shared<FrameGraphVertexShader>("HelloFrameGraphVS", L"BasicShader.hlsl", L"VSMain_P_N_T_UV");
		//pso.m_VSState.m_shader = forward::make_shared<FrameGraphVertexShader>("HelloFrameGraphVS", L"BasicShader.hlsl", L"VSMain");
		pso.m_PSState.m_shader = forward::make_shared<FrameGraphPixelShader>("HelloFrameGraphPS", L"BasicShader.hlsl", L"PSMain");

		pso.m_PSState.m_shaderResources[0] = make_shared<FrameGraphTexture2D>("DDS_Tex", L"bricks.dds");
		pso.m_PSState.m_shaderResources[1] = make_shared<FrameGraphTextureCube>("DDS_Cube", L"snowcube1024.dds");
		pso.m_PSState.m_samplers[0] = make_shared<SamplerState>("TexSamp");

		// setup geometry
		//m_geometry = std::make_unique<SimpleGeometry>("BOX", forward::GeometryBuilder<forward::GP_COLOR_BOX>());
		m_geometry = std::make_unique<SimpleGeometry>("Sphere", forward::GeometryBuilder<forward::GP_SPHERE>(1.0f, 15, 20));
		builder << *m_geometry;

		// setup constant buffer
		m_constantBuffer = make_shared<FrameGraphConstantBuffer<Matrix4f>>("CB");
		pso.m_VSState.m_constantBuffers[0] = m_constantBuffer;

		// setup render states
		auto dsPtr = m_pRender2->GetDefaultDS();
		pso.m_OMState.m_depthStencilResource = dsPtr;

		auto rsPtr = m_pRender2->GetDefaultRT();
		pso.m_OMState.m_renderTargetResources[0] = rsPtr;
	},
	[&](Renderer& render) {
		render.DrawIndexed(m_geometry->GetIndexCount());
	});


	return true;
}

void BasicGeometryFrameGraph::OnResize()
{
	Application::OnResize();
}

void BasicGeometryFrameGraph::OnSpace()
{
	mAppPaused = !mAppPaused;
}