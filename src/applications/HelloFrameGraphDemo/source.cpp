#include "Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"

#include "pix3.h"

using namespace forward;

class HelloFrameGraph : public Application
{
public:
	HelloFrameGraph(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"HelloFrameGraph";
#ifdef WINDOWS
		//RenderType = RendererType::Renderer_Forward_DX11;
		DeviceType = DeviceType::Device_Forward_DX12;
#endif
	}

	~HelloFrameGraph()
	{
		SAFE_DELETE(m_renderPass);
	}

    bool Init() override;
    void OnResize() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void PostDrawScene() override;
	//void OnSpace() override;

private:

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	RenderPass* m_renderPass;
};

void HelloFrameGraph::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void HelloFrameGraph::DrawScene()
{
	PIXScopedEvent(PIX_COLOR_DEFAULT, "DrawScene");
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Blue);
	m_pDevice->EndDrawFrameGraph();
}

bool HelloFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = new RenderPass(
	[&](RenderPassBuilder& /*builder*/, PipelineStateObject& pso) {
		// setup shaders
        pso.m_VSState.m_shader = make_shared<VertexShader>("HelloFrameGraphVS", L"BasicShader", L"VSMainQuad");
        pso.m_PSState.m_shader = make_shared<PixelShader>("HelloFrameGraphPS", L"BasicShader", L"PSMainQuad");
        
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

		auto vb = forward::make_shared<VertexBuffer>("VertexBuffer", vf, 4);
		for (auto i = 0; i < sizeof(quadVertices) / sizeof(Vertex_POS_COLOR); ++i)
		{
			vb->AddVertex(quadVertices[i]);
		}
		vb->SetUsage(ResourceUsage::RU_IMMUTABLE);
		pso.m_IAState.m_vertexBuffers[0] = vb;

		// setup render states
		auto dsPtr = m_pDevice->GetDefaultDS();
		pso.m_OMState.m_depthStencilResource = dsPtr;

		auto rsPtr = m_pDevice->GetDefaultRT();
		pso.m_OMState.m_renderTargetResources[0] = rsPtr;
	},
	[](Device& render) {
		render.Draw(4);
	});


	return true;
}

void HelloFrameGraph::OnResize()
{
	Application::OnResize();
}

//void HelloFrameGraph::OnSpace()
//{
//    mAppPaused = !mAppPaused;
//    m_pRender2->SaveRenderTarget(L"FirstRenderTargetOut.bmp", m_renderPass->GetPSO());
//}

void HelloFrameGraph::PostDrawScene()
{
	m_pDevice->SaveRenderTarget(L"OffScreenRenderingResultDX12.bmp", m_renderPass->GetPSO());
}


FORWARD_APPLICATION_MAIN(HelloFrameGraph, 1920, 1080);
