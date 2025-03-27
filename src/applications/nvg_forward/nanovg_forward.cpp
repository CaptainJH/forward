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

void SetupBlenderStateNoWrite(OutputMergerStageState& om_state) {
	om_state.m_blendState = forward::BlendState();
	om_state.m_blendState.target[0].mask = 0;
}

void SetupBlenderStateBlend(OutputMergerStageState& om_state) {
	om_state.m_blendState = forward::BlendState();
	om_state.m_blendState.target[0].enable = true;
	om_state.m_blendState.target[0].dstColor = forward::BlendState::BM_INV_SRC_ALPHA;
	om_state.m_blendState.target[0].dstAlpha = forward::BlendState::BM_INV_SRC_ALPHA;
}

void SetupDepthStencilDrawShapes(OutputMergerStageState& om_state) {
	auto& dsState = om_state.m_dsState;
	dsState = forward::DepthStencilState();
	dsState.depthEnable = false;
	dsState.stencilEnable = true;
	dsState.frontFace.pass = forward::DepthStencilState::OP_INCR;
	dsState.backFace.pass = forward::DepthStencilState::OP_DECR;
}

void SetupDepthStencilDrawAA(OutputMergerStageState& om_state) {
	auto& dsState = om_state.m_dsState;
	dsState = forward::DepthStencilState();
	dsState.depthEnable = false;
	dsState.stencilEnable = true;
	dsState.frontFace.comparison = forward::DepthStencilState::EQUAL;
	dsState.backFace.comparison = forward::DepthStencilState::EQUAL;
}

void SetupDepthStencilFill(OutputMergerStageState& om_state) {
	auto& dsState = om_state.m_dsState;
	dsState = forward::DepthStencilState();
	dsState.depthEnable = false;
	dsState.stencilEnable = true;
	dsState.frontFace.comparison = forward::DepthStencilState::NOT_EQUAL;
	dsState.frontFace.fail = forward::DepthStencilState::OP_ZERO;
	dsState.frontFace.pass = forward::DepthStencilState::OP_ZERO;
	dsState.backFace.comparison = forward::DepthStencilState::NOT_EQUAL;
	dsState.backFace.fail = forward::DepthStencilState::OP_ZERO;
	dsState.backFace.pass = forward::DepthStencilState::OP_ZERO;
}

void SetupDepthStencilDefault(OutputMergerStageState& om_state) {
	auto& dsState = om_state.m_dsState;
	dsState = forward::DepthStencilState();
	dsState.depthEnable = false;
	dsState.stencilEnable = false;
}

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
		nvgDeleteForward(vg);
	}

    bool Init() override;

protected:
	void UpdateScene(f32) override {}
	void DrawScene() override;
	void OnSpace() override { m_blowup = m_blowup ? 0 : 1; }

	void DrawNVG();
	void SetupRenderPass(RenderPass&, RenderItem&);

private:
	RenderPass* m_renderPass;
	NVGcontext* vg = nullptr;
	DemoData data;
	u32 m_current_ps_cb_index = 0;
	u32 m_next_available_pass_index = 0U;
	std::vector<RenderPass> m_nvg_fill_pass;
	shared_ptr<ConstantBuffer<VS_CONSTANTS>> m_nvg_vs_cb;
	std::vector<shared_ptr<ConstantBuffer<D3DNVGfragUniforms>>> m_nvg_ps_cb;
	shared_ptr<VertexShader> m_nvg_vs;
	shared_ptr<PixelShader> m_nvg_ps;
	shared_ptr<VertexBuffer> m_nvg_vb;
	shared_ptr<IndexBuffer> m_nvg_ib;
	shared_ptr<Texture2D> m_default_tex;
	VertexFormat m_nvg_vf;

	shared_ptr<Texture2D> m_depthStencil;
	shared_ptr<Texture2D> m_rt;

	std::vector<shared_ptr<RasterPipelineStateObject>> m_pso;
	int m_blowup = 0;
};

void nanovg_forward_demo::DrawNVG() {
	const auto xWin = mClientWidth;
	const auto yWin = mClientHeight;
	float xm = 0.0f, ym = 0.0f;
	SDL_GetMouseState(&xm, &ym);
	nvgBeginFrame(vg, xWin, yWin, 1.0f);

	renderDemo(vg, xm, ym, (float)xWin, (float)yWin, mTimer.Runtime(), m_blowup, &data);

	nvgEndFrame(vg);
}

void nanovg_forward_demo::DrawScene()
{
	DrawNVG();

	ProfilingHelper::BeginPixEvent("DrawScene", 0, 200, 0);
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass);
	for (auto i = 0U; i < m_next_available_pass_index; ++i) {
		auto& pass = m_nvg_fill_pass[i];
		fg.DrawRenderPass(&pass);
	}
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Blue);
	m_pDevice->EndDrawFrameGraph();
	ProfilingHelper::EndPixEvent();

	m_nvg_vb->ResetActiveNumElements();
	m_nvg_ib->ResetActiveNumElements();
	m_current_ps_cb_index = 0;
	m_next_available_pass_index = 0;
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

	// load demo data
	{
		auto getNVGFileFullPath = [](const char* file)->std::string {
			const auto dataPath = TextHelper::ToAscii(FileSystem::getSingleton().GetDataFolder());
			return dataPath + file;
			};
		
		for (int i = 0; i < 12; i++) {
			std::stringstream ss;
			ss << "/nvg_data/images/image" << (i + 1) << ".jpg";
			const std::string fileStr = getNVGFileFullPath(ss.str().c_str());
			const auto file = fileStr.c_str();
			data.images[i] = nvgCreateImage_Forward(vg, file);
			if (data.images[i] == 0) {
				printf("Could not load %s.\n", file);
				return false;
			}
		}

		std::string fontFileStr = getNVGFileFullPath("/nvg_data/entypo.ttf");
		data.fontIcons = nvgCreateFont(vg, "icons", fontFileStr.c_str());
		if (data.fontIcons == -1) {
			printf("Could not add font icons.\n");
			return false;
		}
		fontFileStr = getNVGFileFullPath("/nvg_data/Roboto-Regular.ttf");
		data.fontNormal = nvgCreateFont(vg, "sans", fontFileStr.c_str());
		if (data.fontNormal == -1) {
			printf("Could not add font italic.\n");
			return false;
		}
		fontFileStr = getNVGFileFullPath("/nvg_data/Roboto-Bold.ttf");
		data.fontBold = nvgCreateFont(vg, "sans-bold", fontFileStr.c_str());
		if (data.fontBold == -1) {
			printf("Could not add font bold.\n");
			return false;
		}
		fontFileStr = getNVGFileFullPath("/nvg_data/NotoEmoji-Regular.ttf");
		data.fontEmoji = nvgCreateFont(vg, "emoji", fontFileStr.c_str());
		if (data.fontEmoji == -1) {
			printf("Could not add font emoji.\n");
			return false;
		}
		nvgAddFallbackFontId(vg, data.fontNormal, data.fontEmoji);
		nvgAddFallbackFontId(vg, data.fontBold, data.fontEmoji);
	}

	m_renderPass = new RenderPass(
	[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
		// setup shaders
        pso.m_VSState.m_shader = make_shared<VertexShader>("nanovg_forward_demoVS", L"BasicShader", "VSMainQuad");
        pso.m_PSState.m_shader = make_shared<PixelShader>("nanovg_forward_demoPS", L"BasicShader", "PSMainQuad");
        
		// setup geometry
		auto& vf = pso.m_IAState.m_vertexLayout;
		vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
		vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

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
		builder.GetRenderPass()->m_ia_params.m_topologyType = PT_TRIANGLESTRIP;

		// setup render states
		auto dsPtr = m_pDevice->GetDefaultDS();
		builder.GetRenderPass()->m_om_params.m_depthStencilResource = dsPtr;

		auto rsPtr = m_pDevice->GetDefaultRT();
		builder.GetRenderPass()->m_om_params.m_renderTargetResources[0] = rsPtr;

		pso.m_rsState.frontCCW = false;
		},
		[](CommandList& cmdList, RenderPass&) {
			cmdList.Draw(4);
	});

	m_pDevice->EnableImGUI(false);

	m_nvg_fill_pass.reserve(1000);
	RenderItem::AddVertexFunc = [&](const nvg_vertex& v) { m_nvg_vb->AddVertex(v); };
	RenderItem::AddIndexFunc = [&](const u32& idx) { m_nvg_ib->AddIndex(idx); };
	RenderItem::GetCurrentVBActive = [&]() { return m_nvg_vb->GetActiveNumElements(); };
	RenderItem::GetCurrentIBActive = [&]() { return m_nvg_ib->GetActiveNumElements(); };

	m_nvg_vs = make_shared<VertexShader>("nanovg_forward_demoVS", L"D3D11VertexShader", "D3D11VertexShader_Main");
	m_nvg_ps = make_shared<PixelShader>("nanovg_forward_demoPS", L"D3D11PixelShader", "D3D11PixelShader_Main");
	m_nvg_vs_cb = make_shared<ConstantBuffer<VS_CONSTANTS>>("nvg_vs_cb");

	m_nvg_vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
	m_nvg_vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);
	m_nvg_vf.Bind(VASemantic::VA_TEXCOORD, DataFormatType::DF_R32G32_FLOAT, 0);

	m_nvg_vb = make_shared<VertexBuffer>("VertexBuffer", m_nvg_vf, NVG_MaxBufferSize);
	m_nvg_vb->SetUsage(ResourceUsage::RU_DYNAMIC_UPDATE);
	m_nvg_ib = make_shared<IndexBuffer>("IndexBuffer", PT_TRIANGLELIST, NVG_MaxBufferSize);
	m_nvg_ib->SetUsage(ResourceUsage::RU_DYNAMIC_UPDATE);

	m_default_tex = make_shared<Texture2D>("nvg_default_tex", DF_R8G8B8A8_UNORM, 2, 2, TBP_Shader);

	m_pso.push_back(new RasterPipelineStateObject);
	m_pso.back()->m_VSState.m_shader = m_nvg_vs;
	m_pso.back()->m_PSState.m_shader = m_nvg_ps;
	m_pso.back()->m_PSState.m_samplers[0] = make_shared<SamplerState>("SimpleAlbedo_Samp");
	m_pso.back()->m_rsState.cullMode = RasterizerState::CULL_NONE;
	SetupBlenderStateBlend(m_pso.back()->m_OMState);
	SetupDepthStencilDefault(m_pso.back()->m_OMState);

	m_pso.push_back(new RasterPipelineStateObject);
	m_pso.back()->m_VSState.m_shader = m_nvg_vs;
	m_pso.back()->m_PSState.m_shader = m_nvg_ps;
	m_pso.back()->m_PSState.m_samplers[0] = m_pso.front()->m_PSState.m_samplers[0];
	m_pso.back()->m_rsState.cullMode = RasterizerState::CULL_NONE;
	SetupBlenderStateNoWrite(m_pso.back()->m_OMState);
	SetupDepthStencilDrawShapes(m_pso.back()->m_OMState);

	m_pso.push_back(new RasterPipelineStateObject);
	m_pso.back()->m_VSState.m_shader = m_nvg_vs;
	m_pso.back()->m_PSState.m_shader = m_nvg_ps;
	m_pso.back()->m_PSState.m_samplers[0] = m_pso.front()->m_PSState.m_samplers[0];
	m_pso.back()->m_rsState.cullMode = RasterizerState::CULL_NONE;
	SetupBlenderStateBlend(m_pso.back()->m_OMState);
	SetupDepthStencilDrawAA(m_pso.back()->m_OMState);

	m_pso.push_back(new RasterPipelineStateObject);
	m_pso.back()->m_VSState.m_shader = m_nvg_vs;
	m_pso.back()->m_PSState.m_shader = m_nvg_ps;
	m_pso.back()->m_PSState.m_samplers[0] = m_pso.front()->m_PSState.m_samplers[0];
	m_pso.back()->m_rsState.cullMode = RasterizerState::CULL_NONE;
	SetupBlenderStateBlend(m_pso.back()->m_OMState);
	SetupDepthStencilFill(m_pso.back()->m_OMState);

	m_depthStencil = m_pDevice->GetDefaultDS();
	m_rt = m_pDevice->GetDefaultRT();

	forward_nvg_fill_callback = [&](RenderItem& renderItem) {
		if (m_next_available_pass_index >= m_nvg_fill_pass.size()) {
			m_nvg_fill_pass.emplace_back(RenderPass(
				[&](RenderPassBuilder& builder) {
					RenderPass& thisPass = *builder.GetRenderPass();
					SetupRenderPass(thisPass, renderItem);
				},
				[](CommandList& cmdList, RenderPass& pass) {
					if (pass.m_ia_params.m_index_count == 0)
						cmdList.Draw(pass.m_ia_params.m_vertex_count, pass.m_ia_params.m_vertex_start_idx);
					else
						cmdList.DrawIndexed(pass.m_ia_params.m_index_count,
							pass.m_ia_params.m_vertex_start_idx, pass.m_ia_params.m_index_start_idx);
				}, forward::RenderPass::OF_NO_CLEAN));

			m_next_available_pass_index = static_cast<u32>(m_nvg_fill_pass.size());
		}
		else {
			RenderPass& pass = m_nvg_fill_pass[m_next_available_pass_index++];
			SetupRenderPass(pass, renderItem);
		}

		m_nvg_vs_cb->GetTypedData()->viewSize[0] = mClientWidth;
		m_nvg_vs_cb->GetTypedData()->viewSize[1] = mClientHeight;
		};

	return true;
}

void nanovg_forward_demo::SetupRenderPass(RenderPass& thisPass, RenderItem& renderItem) {

	const auto pso_idx = static_cast<u32>(renderItem.pso_type);
	assert(pso_idx >= 0 && pso_idx <= 3);
	thisPass.SetPSO(m_pso[pso_idx]);

	thisPass.m_ia_params.m_vertexBuffers[0]		= m_nvg_vb;
	thisPass.m_ia_params.m_indexBuffer			= renderItem.ib_count == 0 ? nullptr : m_nvg_ib;
	thisPass.m_ia_params.m_topologyType		= renderItem.topologyType;
	thisPass.m_ia_params.m_vertex_count			= renderItem.vb_count;
	thisPass.m_ia_params.m_vertex_start_idx		= renderItem.vb_start_idx;
	thisPass.m_ia_params.m_index_count			= renderItem.ib_count;
	thisPass.m_ia_params.m_index_start_idx		= renderItem.ib_start_idx;

	thisPass.m_vs.m_constantBuffers[0] = m_nvg_vs_cb;
	if (m_current_ps_cb_index >= m_nvg_ps_cb.size()) {
		auto nvg_ps_cb = make_shared<ConstantBuffer<D3DNVGfragUniforms>>("nvg_ps_cb");
		m_nvg_ps_cb.push_back(nvg_ps_cb);
		*nvg_ps_cb = renderItem.constant_buffer;
		thisPass.m_ps.m_constantBuffers[1] = nvg_ps_cb;
	}
	else {
		*m_nvg_ps_cb[m_current_ps_cb_index] = renderItem.constant_buffer;
		thisPass.m_ps.m_constantBuffers[1] = m_nvg_ps_cb[m_current_ps_cb_index];
	}
	++m_current_ps_cb_index;
	if (renderItem.tex)
		thisPass.m_ps.m_shaderResources[0] = renderItem.tex;
	else
		thisPass.m_ps.m_shaderResources[0] = m_default_tex;

	// setup render states
	thisPass.m_om_params.m_depthStencilResource = m_depthStencil;
	thisPass.m_om_params.m_renderTargetResources[0] = m_rt;
}

FORWARD_APPLICATION_MAIN(nanovg_forward_demo, 1000, 600);
