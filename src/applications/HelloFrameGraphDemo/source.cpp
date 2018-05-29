#include "ApplicationWin.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"

using namespace forward;

class HelloFrameGraph : public Application
{
public:
	HelloFrameGraph(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
	{
		mMainWndCaption = L"HelloFrameGraph";
		RenderType = RendererType::Renderer_Forward_DX11;
	}

	~HelloFrameGraph()
	{
		SAFE_DELETE(m_renderPass);
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override;

private:

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	RenderPass* m_renderPass;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HelloFrameGraph theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void HelloFrameGraph::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void HelloFrameGraph::DrawScene()
{
	FrameGraph fg;
	m_pRender2->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass);
	m_pRender2->EndDrawFrameGraph();
}

bool HelloFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = new RenderPass(
	[](RenderPassBuilder& /*builder*/, PipelineStateObject& pso) {
		// setup shaders
		pso.m_VSState.m_shader = forward::make_shared<FrameGraphVertexShader>("HelloFrameGraphVS", L"BasicShader.hlsl", L"VSMainQuad");
		pso.m_PSState.m_shader = forward::make_shared<FrameGraphPixelShader>("HelloFrameGraphPS", L"BasicShader.hlsl", L"PSMainQuad");
	
		// setup geometry
		auto& vf = pso.m_IAState.m_vertexLayout;
		vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
		vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

		pso.m_IAState.m_topologyType = PT_TRIANGLESTRIP;

		/////////////
		///build quad
		/////////////
		Vertex_POS_COLOR quadVertices[] =
		{
			{ Vector3f(-1.0f, +1.0f, 0.0f), Colors::White },
			{ Vector3f(+1.0f, +1.0f, 0.0f), Colors::Red },
			{ Vector3f(-1.0f, -1.0f, 0.0f), Colors::Green },
			{ Vector3f(+1.0f, -1.0f, 0.0f), Colors::Blue }
		};

		auto vb = forward::make_shared<FrameGraphVertexBuffer>("VertexBuffer", vf, 4);
		for (auto i = 0; i < sizeof(quadVertices) / sizeof(Vertex_POS_COLOR); ++i)
		{
			vb->AddVertex(quadVertices[i]);
		}
		vb->SetUsage(ResourceUsage::RU_IMMUTABLE);
		pso.m_IAState.m_vertexBuffers[0] = vb;

		// setup render states
		auto dsPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("DefaultDS");
		pso.m_OMState.m_depthStencilResource = dsPtr;

		auto rsPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("DefaultRT");
		pso.m_OMState.m_renderTargetResources[0] = rsPtr;
	},
	[](Renderer& render) {
		render.Draw(4);
	});


	return true;
}

void HelloFrameGraph::OnResize()
{
	Application::OnResize();
}

void HelloFrameGraph::OnSpace()
{
	mAppPaused = !mAppPaused;
	m_pRender2->SaveRenderTarget(L"FirstRenderTargetOut.bmp");
}