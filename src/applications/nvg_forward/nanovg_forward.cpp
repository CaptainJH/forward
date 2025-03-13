#include "utilities/Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"

#include "utilities/ProfilingHelper.h"
#include "utilities/Log.h"

#pragma warning(disable: 4100)  // unreferenced formal parameter
#pragma warning(disable: 4127)  // conditional expression is constant
#pragma warning(disable: 4204)  // nonstandard extension used : non-constant aggregate initializer
#pragma warning(disable: 4706)  // assignment within conditional expression
#pragma warning(disable: 4244) 
#pragma warning(disable: 4101) 
#pragma warning(disable: 4456) 
#pragma warning(disable: 4701) 
#pragma warning(disable: 4189) 

#include "nanovg.h"
#define NANOVG_FORWARD_IMPLEMENTATION
#include "nanovg_forward.h"
#include "demo.h"


using namespace forward;

class nanovg_forward_demo : public Application
{
public:
	nanovg_forward_demo(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"nanovg_forward_demo";
#ifdef WINDOWS
		DeviceType = DeviceType::Device_Forward_DX12;
#endif
	}

	~nanovg_forward_demo()
	{
		SAFE_DELETE(m_renderPass);
	}

    bool Init() override;

protected:
	void UpdateScene(f32) override {}
	void DrawScene() override;
	//void OnSpace() override;

private:
	RenderPass* m_renderPass;
};

void nanovg_forward_demo::DrawScene()
{
	ProfilingHelper::BeginPixEvent("DrawScene", 0, 200, 0);
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Blue);
	m_pDevice->EndDrawFrameGraph();
	ProfilingHelper::EndPixEvent();
}

bool nanovg_forward_demo::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	DemoData data;
	NVGcontext* vg = NULL;

	vg = nvgCreateForward(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (vg == NULL) {
		std::cout << "Could not init nanovg.\n";
		return false;
	}

	//if (loadDemoData(vg, &data) == -1)
	//	return false;

	m_renderPass = new RenderPass(
	[&](RenderPassBuilder& /*builder*/, RasterPipelineStateObject& pso) {
		// setup shaders
        pso.m_VSState.m_shader = make_shared<VertexShader>("nanovg_forward_demoVS", L"BasicShader", "VSMainQuad");
        pso.m_PSState.m_shader = make_shared<PixelShader>("nanovg_forward_demoPS", L"BasicShader", "PSMainQuad");
        
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
			{ float3(-1.0f, +1.0f, 0.0f), Colors::White },
			{ float3(+1.0f, +1.0f, 0.0f), Colors::Red },
			{ float3(-1.0f, -1.0f, 0.0f), Colors::Green },
			{ float3(+1.0f, -1.0f, 0.0f), Colors::Blue }
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

		pso.m_RSState.m_rsState.frontCCW = false;
		},
		[](CommandList& cmdList) {
			cmdList.Draw(4);
	});

	return true;
}

//void nanovg_forward_demo::OnSpace()
//{
//    mAppPaused = !mAppPaused;
//    m_pRender2->SaveRenderTarget(L"FirstRenderTargetOut.bmp", m_renderPass->GetPSO());
//}


FORWARD_APPLICATION_MAIN(nanovg_forward_demo, 1920, 1080);
