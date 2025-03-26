
#include "utilities/Application.h"
#include "RHI/FrameGraph/Geometry.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "utilities/Log.h"

using namespace forward;

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};

class HelloRaytracing : public Application
{
public:
	HelloRaytracing(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Hello Raytracing!";
	}

	~HelloRaytracing()
	{
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!Application::Init())
			return false;

		m_rayGenCB.viewport = { -1, -1, 1, 1 };
		const f32 border = 0.1f;
		m_rayGenCB.stencil = {
			-1 + border / AspectRatio(), -1 + border,
			 1 - border / AspectRatio(), 1.0f - border
		};

		// setup geometry
		m_ib = make_shared<IndexBuffer>("IndexBuffer", PT_TRIANGLELIST, 3);
		m_ib->AddFace(TriangleIndices(0, 1, 2));

		const f32 depthValue = 1.0;
		const f32 offset = 0.7f;
		Vertex_POS vertices[] =
		{
			// The sample raytraces in screen space coordinates.
			// Since DirectX screen space coordinates are right handed (i.e. Y axis points down).
			// Define the vertices in counter clockwise order ~ clockwise in left handed.
			float3 { 0, -offset, depthValue },
			float3 { -offset, offset, depthValue },
			float3 { offset, offset, depthValue }
		};
		m_vb = make_shared<VertexBuffer>("VertexBuffer", Vertex_POS::GetVertexFormat(), 3);
		m_vb->AddVertex(vertices[0]);
		m_vb->AddVertex(vertices[1]);
		m_vb->AddVertex(vertices[2]);

		m_cb = make_shared<ConstantBuffer<RayGenConstantBuffer>>("g_sceneCB");
		*m_cb = m_rayGenCB;

		m_uavTex = make_shared<Texture2D>("UAV_Tex", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_Shader);
		m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);	

		m_rtPass = std::make_unique<RenderPass>(
			[&](RenderPassBuilder&, RTPipelineStateObject& pso) {
				pso.m_meshes.emplace_back(std::make_pair(m_vb, m_ib));
				pso.m_rtState.m_uavShaderRes[0] = m_uavTex;
				pso.m_rtState.m_constantBuffers[0] = m_cb;

				// setup shaders
				pso.m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"HelloRaytracing");
				pso.m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RayGenShaderTable", L"MyRaygenShader");
				pso.m_rtState.m_hitShaderTable = make_shared<ShaderTable>("HitGroupShaderTable", L"HitGroup_MyClosestHitShader");
				pso.m_rtState.m_missShaderTable = make_shared<ShaderTable>("MissShaderTable", L"MyMissShader");
			},
			[&](CommandList& cmdList, RenderPass&) {
				auto& rtPSO = m_rtPass->GetPSO<RTPipelineStateObject>();
				cmdList.DispatchRays(rtPSO);
				cmdList.CopyResource(*m_pDevice->GetCurrentSwapChainRT(), *m_uavTex);
			}
		);

		return true;
	}

protected:
	void UpdateScene(f32) override {}

	void DrawScene() override
	{
		FrameGraph fg;
		m_pDevice->BeginDrawFrameGraph(&fg);
		fg.DrawRenderPass(m_rtPass.get());
		m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
		m_pDevice->EndDrawFrameGraph();
	}

	shared_ptr<IndexBuffer> m_ib;
	shared_ptr<VertexBuffer> m_vb;
	shared_ptr<ConstantBuffer<RayGenConstantBuffer>> m_cb;
	shared_ptr<Texture2D> m_uavTex;
	std::unique_ptr<RenderPass> m_rtPass;

	// Raytracing scene
	RayGenConstantBuffer m_rayGenCB;
};

FORWARD_APPLICATION_MAIN(HelloRaytracing, 1920, 1080);