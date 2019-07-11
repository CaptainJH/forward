#include "UnityApplication.h"

using namespace forward;

bool UnityApplication::Init()
{
	Log::Get().Open();

	m_renderPass = new RenderPass(
		[&](RenderPassBuilder& /*builder*/, PipelineStateObject& pso) {
			// setup shaders
			pso.m_VSState.m_shader = make_shared<FrameGraphVertexShader>("HelloFrameGraphVS", L"BasicShader", L"VSMainQuad");
			pso.m_PSState.m_shader = make_shared<FrameGraphPixelShader>("HelloFrameGraphPS", L"BasicShader", L"PSMainQuad");

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
			auto dsPtr = m_pRender2->GetDefaultDS();
			pso.m_OMState.m_depthStencilResource = dsPtr;

			auto rsPtr = m_pRender2->GetDefaultRT();
			pso.m_OMState.m_renderTargetResources[0] = rsPtr;
		},
		[](Renderer& render) {
			render.Draw(4);
		});


	return true;
}

void UnityApplication::OnResize()
{

}

void UnityApplication::UpdateScene(f32 /*dt*/)
{

}

void UnityApplication::DrawScene()
{
	FrameGraph fg;
	m_pRender2->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass);
	m_pRender2->EndDrawFrameGraph();
}