#include "ApplicationWin.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
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
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override;

private:

	Matrix4f m_WVPMat;
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
	m_WVPMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
	*m_constantBuffer = m_WVPMat * m_viewMat * m_projMat;
}

void BasicGeometryFrameGraph::DrawScene()
{
	m_pRender2->DrawRenderPass(*m_renderPass);
}

bool BasicGeometryFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = std::make_unique<RenderPass>(RenderPass::CT_Default, 
	[&](RenderPassBuilder& builder, PipelineStateObject& pso) {
		// Init the world matrix
		m_WVPMat = Matrix4f::Identity();
		// Build the view matrix.
		Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
		Vector3f target; target.MakeZero();
		Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
		m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
		// Build the projection matrix
		m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);

		// setup shaders
		pso.m_VSState.m_shader = forward::make_shared<FrameGraphVertexShader>("HelloFrameGraphVS", L"BasicShader.hlsl", L"VSMain");
		pso.m_PSState.m_shader = forward::make_shared<FrameGraphPixelShader>("HelloFrameGraphPS", L"BasicShader.hlsl", L"PSMain");

		// setup geometry
		VertexFormat vf;
		vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
		vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

		m_geometry = std::make_unique<SimpleGeometry>("SimpleGeometry", vf, PT_TRIANGLELIST, 8, 12 * 3);
		m_geometry->AddVertex(Vertex{ Vector3f(-1.0f, -1.0f, -1.0f), Colors::White });
		m_geometry->AddVertex(Vertex{ Vector3f(-1.0f, +1.0f, -1.0f), Colors::Black });
		m_geometry->AddVertex(Vertex{ Vector3f(+1.0f, +1.0f, -1.0f), Colors::Red });
		m_geometry->AddVertex(Vertex{ Vector3f(+1.0f, -1.0f, -1.0f), Colors::Green });
		m_geometry->AddVertex(Vertex{ Vector3f(-1.0f, -1.0f, +1.0f), Colors::Blue });
		m_geometry->AddVertex(Vertex{ Vector3f(-1.0f, +1.0f, +1.0f), Colors::Yellow });
		m_geometry->AddVertex(Vertex{ Vector3f(+1.0f, +1.0f, +1.0f), Colors::Cyan });
		m_geometry->AddVertex(Vertex{ Vector3f(+1.0f, -1.0f, +1.0f), Colors::Magenta });

		m_geometry->AddFace(TriangleIndices(0, 1, 2));
		m_geometry->AddFace(TriangleIndices(0, 2, 3));

		m_geometry->AddFace(TriangleIndices(4, 6, 5));
		m_geometry->AddFace(TriangleIndices(4, 7, 6));

		m_geometry->AddFace(TriangleIndices(4, 5, 1));
		m_geometry->AddFace(TriangleIndices(4, 1, 0));

		m_geometry->AddFace(TriangleIndices(3, 2, 6));
		m_geometry->AddFace(TriangleIndices(3, 6, 7));

		m_geometry->AddFace(TriangleIndices(1, 5, 6));
		m_geometry->AddFace(TriangleIndices(1, 6, 2));

		m_geometry->AddFace(TriangleIndices(4, 0, 3));
		m_geometry->AddFace(TriangleIndices(4, 3, 7));
		// end setup geometry

		builder << *m_geometry;

		// setup constant buffer
		m_constantBuffer = make_shared<FrameGraphConstantBuffer<Matrix4f>>("CB");
		m_constantBuffer->SetUsage(RU_DYNAMIC_UPDATE); // set to dynamic so we can update the buffer every frame.
		pso.m_VSState.m_constantBuffers[0] = m_constantBuffer;

		// setup render states
		auto dsPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("DefaultDS");
		pso.m_OMState.m_depthStencilResource = dsPtr;

		auto rsPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("DefaultRT");
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