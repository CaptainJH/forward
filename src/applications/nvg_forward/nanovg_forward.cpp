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

#include <SDL3/SDL.h>

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

	void DrawNVG();

private:
	RenderPass* m_renderPass;
	NVGcontext* vg = nullptr;
	DemoData data;
	std::vector<RenderPass*> m_nvg_fill_pass;
	shared_ptr<ConstantBuffer<VS_CONSTANTS>> m_nvg_vs_cb;
	std::vector<shared_ptr<ConstantBuffer<D3DNVGfragUniforms>>> m_nvg_ps_cb;
};

void nanovg_forward_demo::DrawNVG() {
	const auto xWin = mClientWidth;
	const auto yWin = mClientHeight;
	float xm = 0.0f, ym = 0.0f;
	SDL_GetMouseState(&xm, &ym);
	nvgBeginFrame(vg, xWin, yWin, 1.0f);

	renderDemo(vg, xm, ym, (float)xWin, (float)yWin, 1.0f, 0, &data);

	nvgEndFrame(vg);
}

void nanovg_forward_demo::DrawScene()
{
	DrawNVG();

	ProfilingHelper::BeginPixEvent("DrawScene", 0, 200, 0);
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass);
	for (auto rpass : m_nvg_fill_pass)
		fg.DrawRenderPass(rpass);
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Blue);
	m_pDevice->EndDrawFrameGraph();
	ProfilingHelper::EndPixEvent();
}

bool nanovg_forward_demo::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

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

	forward_nvg_fill_callback = [&](std::vector<nvg_vertex>& v, std::vector<int>& idx, D3DNVGfragUniforms& uniform) {
		if (m_nvg_fill_pass.size() >= 8) {
			return;
		}

		const auto c = static_cast<u32>(idx.size());

		m_nvg_fill_pass.push_back(new RenderPass(
			[&](RenderPassBuilder& /*builder*/, RasterPipelineStateObject& pso) {
				// setup shaders
				pso.m_VSState.m_shader = make_shared<VertexShader>("nanovg_forward_demoVS", L"D3D11VertexShader", "D3D11VertexShader_Main");
				pso.m_PSState.m_shader = make_shared<PixelShader>("nanovg_forward_demoPS", L"D3D11PixelShader", "D3D11PixelShader_Main");

				// setup geometry
				auto& vf = pso.m_IAState.m_vertexLayout;
				vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
				vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);
				vf.Bind(VASemantic::VA_TEXCOORD, DataFormatType::DF_R32G32_FLOAT, 0);

				pso.m_IAState.m_topologyType = PT_TRIANGLELIST;

				auto vb = forward::make_shared<VertexBuffer>("VertexBuffer", vf, static_cast<u32>(v.size()));
				for (auto i = 0; i < v.size(); ++i)
				{
					vb->AddVertex(v[i]);
				}
				vb->SetUsage(ResourceUsage::RU_IMMUTABLE);
				pso.m_IAState.m_vertexBuffers[0] = vb;

				m_nvg_vs_cb = make_shared<ConstantBuffer<VS_CONSTANTS>>("nvg_vs_cb");
				auto nvg_ps_cb = make_shared<ConstantBuffer<D3DNVGfragUniforms>>("nvg_ps_cb");
				m_nvg_ps_cb.push_back(nvg_ps_cb);
				pso.m_VSState.m_constantBuffers[0] = m_nvg_vs_cb;
				pso.m_PSState.m_constantBuffers[1] = nvg_ps_cb;
				pso.m_PSState.m_shaderResources[0] = make_shared<Texture2D>("nvg_tex", L"bricks.dds");
				pso.m_PSState.m_samplers[0] = forward::make_shared<SamplerState>("SimpleAlbedo_Samp");
				
				auto ib = forward::make_shared<IndexBuffer>("IndexBuffer", PT_TRIANGLELIST, static_cast<u32>(idx.size()));
				for (auto i : idx)
					ib->AddIndex(i);
				pso.m_IAState.m_indexBuffer = ib;

				pso.m_RSState.m_rsState.cullMode = forward::RasterizerState::CULL_NONE;

				// setup render states
				auto dsPtr = m_pDevice->GetDefaultDS();
				pso.m_OMState.m_depthStencilResource = dsPtr;

				auto rsPtr = m_pDevice->GetDefaultRT();
				pso.m_OMState.m_renderTargetResources[0] = rsPtr;

				pso.m_OMState.m_blendState.target[0].mask = 0;

				auto& dsState = pso.m_OMState.m_dsState;
				dsState.depthEnable = false;
				dsState.stencilEnable = true;
				dsState.frontFace.pass = forward::DepthStencilState::OP_INCR;
				dsState.backFace.pass = forward::DepthStencilState::OP_DECR;
			},
			[=](CommandList& cmdList) {
				cmdList.DrawIndexed(c);
			}, forward::RenderPass::OF_NO_CLEAN));

		m_nvg_vs_cb->GetTypedData()->viewSize[0] = mClientWidth;
		m_nvg_vs_cb->GetTypedData()->viewSize[1] = mClientHeight;
		(*m_nvg_ps_cb.back()->GetTypedData()) = uniform;
		};

	return true;
}

//void nanovg_forward_demo::OnSpace()
//{
//    mAppPaused = !mAppPaused;
//    m_pRender2->SaveRenderTarget(L"FirstRenderTargetOut.bmp", m_renderPass->GetPSO());
//}


FORWARD_APPLICATION_MAIN(nanovg_forward_demo, 1000, 600);
