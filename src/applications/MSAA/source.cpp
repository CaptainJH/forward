#include "ApplicationWin.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"

using namespace forward;

struct CBufferTypeVS
{
	Matrix4f mat;
	Matrix4f dummy;
};

struct CBufferTypePS
{
	Vector4f distance;
	Vector4f colorAA;
};

class MSAA_Demo : public Application
{
public:
	MSAA_Demo(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
	{
		mMainWndCaption = L"MSAA_Demo";
		DeviceType = DeviceType::Device_Forward_DX12;
	}

	~MSAA_Demo()
	{
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override;

private:

	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	shared_ptr<ConstantBuffer<CBufferTypeVS>> m_constantBufferVS;
	shared_ptr<ConstantBuffer<CBufferTypePS>> m_constantBufferPS;
	shared_ptr<Texture2D> m_msaa_rt;
	shared_ptr<Texture2D> m_msaa_ds;
	shared_ptr<Texture2D> m_msaa_resolved;

	std::unique_ptr<SimpleGeometry> m_geometry;
	std::unique_ptr<SimpleGeometry> m_quad;

	std::unique_ptr<RenderPass> m_renderPass;
	std::unique_ptr<RenderPass> m_renderPassMSAA;
	std::unique_ptr<RenderPass> m_renderPassResolve;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	MSAA_Demo theApp(hInstance, 1920, 1080);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void MSAA_Demo::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	auto worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
	(*m_constantBufferVS).GetTypedData()->mat = worldMat * m_viewMat * m_projMat;
}

void MSAA_Demo::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	(*m_constantBufferPS).GetTypedData()->distance = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	fg.DrawRenderPass(m_renderPass.get());
	fg.DrawRenderPass(m_renderPassMSAA.get());
	fg.DrawRenderPass(m_renderPassResolve.get());
	m_pDevice->EndDrawFrameGraph();
}

bool MSAA_Demo::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	// Build the view matrix.
	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
	// Build the projection matrix
	m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * f_PI, AspectRatio(), 0.01f, 100.0f);

	m_renderPass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
			// setup shaders
			pso.m_VSState.m_shader = forward::make_shared<VertexShader>("Pass1VS", L"MSAA_Shader", "VSMain");
			pso.m_PSState.m_shader = forward::make_shared<PixelShader>("Pass1PS", L"MSAA_Shader", "PSMain");
			pso.m_GSState.m_shader = forward::make_shared<GeometryShader>("Pass1GS", L"MSAA_Shader", "GSMain");

			// setup geometry
			m_geometry = std::make_unique<SimpleGeometry>("BOX", forward::GeometryBuilder<forward::GP_COLOR_BOX>());
			builder << *m_geometry;

			// setup constant buffer
			m_constantBufferVS = make_shared<ConstantBuffer<CBufferTypeVS>>("CB0");
			m_constantBufferPS = make_shared<ConstantBuffer<CBufferTypePS>>("CB1");
			pso.m_VSState.m_constantBuffers[0] = m_constantBufferVS;
			pso.m_PSState.m_constantBuffers[1] = m_constantBufferPS;

			// setup render targets
			auto dsPtr = m_pDevice->GetDefaultDS();
			pso.m_OMState.m_depthStencilResource = dsPtr;

			auto rsPtr = m_pDevice->GetDefaultRT();
			pso.m_OMState.m_renderTargetResources[0] = rsPtr;

			// setup rasterizer
			forward::RECT scissorRect = { 0, 0, mClientWidth / 2, mClientHeight };
			pso.m_RSState.AddScissorRect(scissorRect);
			pso.m_RSState.m_rsState.enableScissor = true;
			pso.m_RSState.m_rsState.frontCCW = false;
		},
		[&](CommandList& cmdList) {
			cmdList.DrawIndexed(m_geometry->GetIndexCount());

			// update constant buffer for next pass
			(*m_constantBufferPS).GetTypedData()->colorAA = Vector4f(0.0f, 0.0f, 1.0f, 1.0f);
	});

	m_renderPassMSAA = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
			// setup shaders
			pso.m_VSState.m_shader = GraphicsObject::FindFrameGraphObject<VertexShader>("Pass1VS");
			pso.m_PSState.m_shader = forward::make_shared<PixelShader>("Pass2PS", L"MSAA_Shader", "PSMainAA");
			pso.m_GSState.m_shader = GraphicsObject::FindFrameGraphObject<GeometryShader>("Pass1GS");

			// setup geometry
			builder << *m_geometry;

			// setup constant buffer
			pso.m_VSState.m_constantBuffers[0] = m_constantBufferVS;
			pso.m_PSState.m_constantBuffers[1] = m_constantBufferPS;

			// setup render targets
			m_msaa_rt = make_shared<Texture2D>("MSAA_RT", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_RT, true);
			m_msaa_ds = make_shared<Texture2D>("MSAA_DS", DF_D24_UNORM_S8_UINT, mClientWidth, mClientHeight, TextureBindPosition::TBP_DS, true);
			pso.m_OMState.m_renderTargetResources[0] = m_msaa_rt;
			pso.m_OMState.m_depthStencilResource = m_msaa_ds;
			m_msaa_resolved = make_shared<Texture2D>("Final_RT", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_RT | TextureBindPosition::TBP_Shader);

			pso.m_RSState.m_rsState.frontCCW = false;
		},
			[&](CommandList& cmdList) {
			cmdList.DrawIndexed(m_geometry->GetIndexCount());
			cmdList.ResolveResource(m_msaa_resolved.get(), m_msaa_rt.get());
		});
	//m_renderPass->AttachRenderPass(m_renderPassMSAA.get());

	m_renderPassResolve = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
			// setup shaders
			pso.m_VSState.m_shader = forward::make_shared<VertexShader>("Pass3VS", L"MSAA_Shader", "VSMainQuad");
			pso.m_PSState.m_shader = forward::make_shared<PixelShader>("Pass3PS", L"MSAA_Shader", "PSMainQuad");
			pso.m_PSState.m_shaderResources[0] = m_msaa_resolved;
			pso.m_PSState.m_samplers[0] = forward::make_shared<SamplerState>("QuadSampler");

			// setup geometry
			m_quad = std::make_unique<SimpleGeometry>("Quad", forward::GeometryBuilder<forward::GP_SCREEN_QUAD>());
			builder << *m_quad;

			// setup render targets
			pso.m_OMState.m_renderTargetResources[0] = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultRT");
			pso.m_OMState.m_depthStencilResource = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultDS");

			// setup rasterizer
			auto rsPtr = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultRT");
			forward::RECT scissorRect = { mClientWidth / 2, 0, mClientWidth, mClientHeight };
			pso.m_RSState.AddScissorRect(scissorRect);
			pso.m_RSState.m_rsState.enableScissor = true;
			pso.m_RSState.m_rsState.frontCCW = false;
			},
			[&](CommandList& cmdList) {
				cmdList.Draw(m_quad->GetVertexCount());
		}, RenderPass::OF_NO_CLEAN);
	//m_renderPassMSAA->AttachRenderPass(m_renderPassResolve.get());


	return true;
}

void MSAA_Demo::OnResize()
{
	Application::OnResize();
}

void MSAA_Demo::OnSpace()
{
	mAppPaused = !mAppPaused;
	//m_pRender2->SaveRenderTarget(L"FirstRenderTargetOut.bmp");
}