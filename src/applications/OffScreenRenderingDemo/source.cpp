#include "ApplicationWin.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"
#include <iostream>

#include "D:/Program Files/RenderDoc/renderdoc_app.h"

using namespace forward;

class OffScreenRenderingDemo : public Application
{
public:
	OffScreenRenderingDemo(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"OffScreenRenderingDemo";
	}

	~OffScreenRenderingDemo()
	{
	}

	bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	std::unique_ptr<RenderPass> m_renderPass;
};

RENDERDOC_API_1_2_0 *rdoc_api = nullptr;
i32 main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HMODULE mod = LoadLibraryA("D:/Program Files/RenderDoc/renderdoc.dll");
	if (mod)
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_2_0, (void**)&rdoc_api);
		assert(ret == 1);
	}

	OffScreenRenderingDemo theApp(1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void OffScreenRenderingDemo::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void OffScreenRenderingDemo::DrawScene()
{
	rdoc_api->StartFrameCapture(NULL, NULL);
	FrameGraph fg;
	m_pRender2->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass.get());
	m_pRender2->EndDrawFrameGraph();
	rdoc_api->EndFrameCapture(NULL, NULL);
}

bool OffScreenRenderingDemo::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, PipelineStateObject& pso) {
		// setup shaders
		pso.m_VSState.m_shader = forward::make_shared<FrameGraphVertexShader>("OffScreenRenderingDemoVS", L"BasicShader.hlsl", L"VSMainQuad");
		pso.m_PSState.m_shader = forward::make_shared<FrameGraphPixelShader>("OffScreenRenderingDemoPS", L"BasicShader.hlsl", L"PSMainQuad");

		// setup geometry
		auto geometry = std::make_unique<SimpleGeometry>("Geometry", forward::GeometryBuilder<forward::GP_SCREEN_QUAD>());
		builder << *geometry;

		// setup render targets
		auto rtPtr = forward::make_shared<FrameGraphTexture2D>(std::string("DefaultRT"), DF_R8G8B8A8_UNORM,
			mClientWidth, mClientHeight, TextureBindPosition::TBP_RT);
		rtPtr->SetUsage(ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL);
		pso.m_OMState.m_renderTargetResources[0] = rtPtr;

		auto dsPtr = forward::make_shared<FrameGraphTexture2D>(std::string("DefaultDS"), DF_D32_FLOAT,
			mClientWidth, mClientHeight, TextureBindPosition::TBP_DS);
		pso.m_OMState.m_depthStencilResource = dsPtr;
	},
		[](Renderer& render) {
		render.Draw(4);
		render.SaveRenderTarget(L"OffScreenRenderingResult.bmp");
	});


	return true;
}