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
		, m_vsID(-1)
		, m_psID(-1)
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
	void SetupPipeline();
	void BuildFrameGraph();

	i32 m_vsID;
	i32 m_psID;

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;
	ResourcePtr m_constantBuffer;

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

	BuildFrameGraph();
}

void BasicGeometryFrameGraph::DrawScene()
{

}

bool BasicGeometryFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;



	return true;
}

void BasicGeometryFrameGraph::BuildShaders()
{

}

void BasicGeometryFrameGraph::OnResize()
{
	Application::OnResize();
	SetupPipeline();
}

void BasicGeometryFrameGraph::BuildGeometry()
{
	
}

void BasicGeometryFrameGraph::SetupPipeline()
{
}

void BasicGeometryFrameGraph::OnSpace()
{
	mAppPaused = !mAppPaused;
}

void BasicGeometryFrameGraph::BuildFrameGraph()
{


}

void MyRenderPass::Execute()
{
	//RendererDX11* renderDX11 = dynamic_cast<RendererDX11*>(m_render);
	
}