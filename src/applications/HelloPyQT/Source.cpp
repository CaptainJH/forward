#include <SDKDDKVer.h>
#include <iostream>
#include "HelloPyQT.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"

using namespace forward;

class HelloPyQTDll : public Application
{
public:
	HelloPyQTDll(HWND hwnd, i32 width, i32 height)
		: Application(hwnd, width, height)
	{}

	~HelloPyQTDll()
	{}

	bool Init() override;

protected:
	void UpdateScene(f32) override {}
	void DrawScene() override;

private:
	std::unique_ptr<RenderPass> m_renderPass;
};

bool HelloPyQTDll::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = std::make_unique<RenderPass>(
		[&]([[maybe_unused]]RenderPassBuilder& builder, PipelineStateObject& pso) {

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
		[](Device& device) {
			device.GetCmdList().Draw(4);
	});

	return true;
}

void HelloPyQTDll::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass.get());
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Blue);
	m_pDevice->EndDrawFrameGraph();
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/,
	DWORD  ul_reason_for_call,
	LPVOID /*lpReserved*/
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HelloPyQTDll* gApp = nullptr;

void Forward_Constructor(HWND hwnd, int w, int h)
{
	if (!gApp)
	{
		//::MessageBoxA(hwnd, "Boom", "Boom", MB_OK);

		gApp = new HelloPyQTDll(hwnd, w, h);

		if (!gApp->Init())
			return;
	}
}

void Forward_Update()
{
	if (gApp)
	{
		gApp->Run();
	}
}

void Forward_Destructor()
{
	SAFE_DELETE(gApp);
}