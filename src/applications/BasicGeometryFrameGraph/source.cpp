#include "ApplicationWin.h"
#include "TriangleIndices.h"
#include "render/FrameGraph/FrameGraph.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
};

// structure for constant buffer
struct CBufferType
{
	Matrix4f mat;
};

class MyRenderPass : public RenderPass
{
public:
	void Execute() override;

	Renderer* m_render = nullptr;
};

class BasicGeometryFrameGraph : public Application
{
public:
	BasicGeometryFrameGraph(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
	{
		mMainWndCaption = L"BasicGeometryFrameGraph";
		RenderType = RendererType::Renderer_Forward_DX11;
	}

	~BasicGeometryFrameGraph()
	{
		Log::Get().Close();
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	virtual void UpdateScene(f32 dt);
	virtual void DrawScene();
	virtual void OnSpace();

private:
	void BuildShaders();
	void BuildGeometry();
	void SetupPipelineStates();

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	MyRenderPass m_renderPass;
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
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void BasicGeometryFrameGraph::DrawScene()
{
	m_pRender2->DrawRenderPass(m_renderPass);
}

bool BasicGeometryFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	BuildShaders();
	BuildGeometry();
	SetupPipelineStates();

	return true;
}

void BasicGeometryFrameGraph::BuildShaders()
{
	m_renderPass.GetPSO().m_VSState.m_shader = new FrameGraphVertexShader("HelloFrameGraphVS", L"BasicShader.hlsl", L"VSMainQuad");
	m_renderPass.GetPSO().m_PSState.m_shader = new FrameGraphPixelShader("HelloFrameGraphPS", L"BasicShader.hlsl", L"PSMainQuad");
}

void BasicGeometryFrameGraph::OnResize()
{
	Application::OnResize();
}

void BasicGeometryFrameGraph::BuildGeometry()
{
	auto& vf = m_renderPass.GetPSO().m_IAState.m_vertexLayout;
	vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
	vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

	m_renderPass.GetPSO().m_IAState.m_topologyType = PT_TRIANGLESTRIP;

	/////////////
	///build quad
	/////////////
	Vertex quadVertices[] =
	{
		{ Vector3f(-1.0f, +1.0f, 0.0f), Colors::White },
		{ Vector3f(+1.0f, +1.0f, 0.0f), Colors::Red },
		{ Vector3f(-1.0f, -1.0f, 0.0f), Colors::Green },
		{ Vector3f(+1.0f, -1.0f, 0.0f), Colors::Blue }
	};

	auto vb = new FrameGraphVertexBuffer("VertexBuffer", vf, 4);
	for (auto i = 0; i < sizeof(quadVertices) / sizeof(Vertex); ++i)
	{
		vb->AddVertex(quadVertices[i]);
	}
	vb->SetUsage(ResourceUsage::RU_IMMUTABLE);
	m_renderPass.GetPSO().m_IAState.m_vertexBuffers[0] = vb;
}

void BasicGeometryFrameGraph::SetupPipelineStates()
{
	auto dsPtr = m_pRender2->FindFrameGraphObject("DefaultDS");
	m_renderPass.GetPSO().m_OMState.m_depthStencilResource =
		static_cast<FrameGraphTexture2D*>(dsPtr);

	auto rsPtr = m_pRender2->FindFrameGraphObject("DefaultRT");
	m_renderPass.GetPSO().m_OMState.m_renderTargetResources[0] =
		static_cast<FrameGraphTexture2D*>(rsPtr);
}

void BasicGeometryFrameGraph::OnSpace()
{
	mAppPaused = !mAppPaused;
}

void MyRenderPass::Execute()
{
	//RendererDX11* renderDX11 = dynamic_cast<RendererDX11*>(m_render);
	
}