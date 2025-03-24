#include "utilities/Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"

#include "utilities/ProfilingHelper.h"
#include "utilities/Log.h"

using namespace forward;

class HelloFrameGraph : public Application
{
public:
	HelloFrameGraph(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"HelloFrameGraph";
#ifdef WINDOWS
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
	void UpdateScene(f32) override {}
	void DrawScene() override;
	void PostDrawScene() override;
	//void OnSpace() override;

private:
	RenderPass* m_renderPass;

	std::unique_ptr<RenderPass> m_computePass;
	forward::shared_ptr<Texture2D> m_uavTex;
};

void HelloFrameGraph::DrawScene()
{
	ProfilingHelper::BeginPixEvent("DrawScene", 0, 200, 0);
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_computePass.get());
	fg.DrawRenderPass(m_renderPass);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Blue);
	m_pDevice->EndDrawFrameGraph();
	ProfilingHelper::EndPixEvent();
}

bool HelloFrameGraph::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = new RenderPass(
	[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
		// setup shaders
        pso.m_VSState.m_shader = make_shared<VertexShader>("HelloFrameGraphVS", L"BasicShader", "VSMainQuad");
        pso.m_PSState.m_shader = make_shared<PixelShader>("HelloFrameGraphPS", L"BasicShader", "PSMainQuad");
        
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
		builder.GetRenderPass()->m_ia_params.m_vertexBuffers[0] = vb;

		// setup render states
		auto dsPtr = m_pDevice->GetDefaultDS();
		builder.GetRenderPass()->m_om_params.m_depthStencilResource = dsPtr;

		auto rsPtr = m_pDevice->GetDefaultRT();
		builder.GetRenderPass()->m_om_params.m_renderTargetResources[0] = rsPtr;

		pso.m_rsState.frontCCW = false;
		},
		[](CommandList& cmdList) {
			cmdList.Draw(4);
	});


	m_uavTex = forward::make_shared<Texture2D>("UAV_Tex", forward::DF_B8G8R8A8_UNORM, 1024, 1024, forward::TextureBindPosition::TBP_Shader);
	m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
	m_computePass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, ComputePipelineStateObject& pso) {
			// setup shaders
			pso.m_CSState.m_shader = forward::make_shared<ComputeShader>("PBR_Baker", L"PBRShader", "BakerMain");
			builder.GetRenderPass()->m_cs.m_uavShaderRes[0] = m_uavTex;
		},
		[](CommandList& cmdList) {
			cmdList.Dispatch(64, 64, 1);
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
	m_pDevice->SaveRenderTarget(L"OffScreenRenderingResultDX12.bmp", m_renderPass);
}


FORWARD_APPLICATION_MAIN(HelloFrameGraph, 1920, 1080);
