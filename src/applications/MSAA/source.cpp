#include "dx11/ApplicationDX11.h"
#include "dx11/ResourceSystem/Buffer/BufferConfigDX11.h"
#include "dx11/ResourceSystem/StateObject/RasterizerStateConfigDX11.h"
#include "dx11/ResourceSystem/Texture/Texture2dConfigDX11.h"
#include "dx11/ResourceSystem/StateObject/SamplerStateConfigDX11.h"
#include "TriangleIndices.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
};

// structure for constant buffer
struct CBufferType
{
	Matrix4f mat;
	Vector4f distance;
};

class MSAA_Demo : public Application
{
public:
	MSAA_Demo(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
		, m_vsID(-1)
		, m_psID(-1)
		, m_vsQuadID(-1)
		, m_psQuadID(-1)
		, m_psAAID(-1)
		, m_gsID(-1)
		, m_drawFrame(true)
	{
		mMainWndCaption = L"MSAA_Demo";
	}

	~MSAA_Demo()
	{
		Log::Get().Close();
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	virtual void UpdateScene(f32 dt);
	virtual void DrawScene();
	virtual void OnSpace();
	virtual void OnEnter();

private:
	void BuildShaders();
	void BuildGeometry();
	void BuildRenderTarget();
	void SetupPipeline();

	i32 m_vsID;
	i32 m_psID;

	i32 m_vsQuadID;
	i32 m_psQuadID;

	i32 m_psAAID;

	i32 m_gsID;

	GeometryPtr m_pGeometry;
	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;
	Resource1Ptr m_constantBuffer;

	GeometryPtr m_pQuad;

	i32 m_rsStateID;
	Resource1Ptr m_renderTargetTex;
	Resource1Ptr m_depthTargetTex;
	Resource1Ptr m_resolveTex;
	i32 m_samplerID;

	bool m_drawFrame;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	MSAA_Demo theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void MSAA_Demo::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void MSAA_Demo::DrawScene()
{
	m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

	// Pass 1 : draw a cube without any AA
	{
		CBufferType buffer;
		buffer.mat = m_worldMat * m_viewMat * m_projMat;
		buffer.distance = Vector4f(1.0f, 0.0f, 0.0f, m_drawFrame ? 1.0f : 0.0f);
		m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsID);
		vsState.ConstantBuffers.SetState(0, m_constantBuffer);
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 gsState;
		gsState.ShaderProgram.SetState(m_gsID);
		m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psID);
		psState.ConstantBuffers.SetState(0, m_constantBuffer);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;

		D3D11_RECT rect = { 0, 0, mClientWidth / 2, mClientHeight };
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ScissorRectCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ScissorRects.SetState(0, rect);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.RasterizerState.SetState(m_rsStateID);

		m_pRender->pImmPipeline->ApplyPipelineResources();
		m_pGeometry->Execute(m_pRender->pImmPipeline);
	}

	// Pass 2 : draw to a render target
	{
		CBufferType buffer;
		buffer.mat = m_worldMat * m_viewMat * m_projMat;
		buffer.distance = Vector4f(0.0f, 0.0f, 0.0f, m_drawFrame ? 1.0f : 0.0f);
		m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

		// to render target
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_renderTargetTex);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_depthTargetTex);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

		D3D11_RECT rect = { 0, 0, mClientWidth, mClientHeight };
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ScissorRectCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ScissorRects.SetState(0, rect);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.RasterizerState.SetState(m_rsStateID);

		m_pRender->pImmPipeline->PixelShaderStage.DesiredState.ShaderProgram.SetState(m_psAAID);
		m_pRender->pImmPipeline->ApplyPipelineResources();
		m_pGeometry->Execute(m_pRender->pImmPipeline);

		ResourceDX11* resolveTex = dynamic_cast<ResourceDX11*>(m_resolveTex.get());
		ResourceDX11* renderTarget = dynamic_cast<ResourceDX11*>(m_renderTargetTex.get());
		m_pRender->pImmPipeline->ResolveSubresource(resolveTex, 0, renderTarget, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

		// draw quad
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_RenderTarget);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_DepthTarget);
		m_pRender->pImmPipeline->ApplyRenderTargets();

		rect = { mClientWidth / 2, 0, mClientWidth, mClientHeight };
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ScissorRectCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ScissorRects.SetState(0, rect);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.RasterizerState.SetState(m_rsStateID);

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsQuadID);
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 gsState;
		gsState.ShaderProgram.SetState(-1);
		m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psQuadID);
		psState.SamplerStates.SetState(0, m_pRender->GetSamplerState(m_samplerID).Get());
		psState.ShaderResources.SetState(0, m_resolveTex);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;

		m_pRender->pImmPipeline->ApplyPipelineResources();
		m_pQuad->Execute(m_pRender->pImmPipeline);
	}

	m_pRender->Present(MainWnd(), 0);
}

bool MSAA_Demo::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	// Init the world matrix
	m_worldMat = Matrix4f::Identity();
	// Build the view matrix.
	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
	// Build the projection matrix
	m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);

	ConstantBufferConfig cbConfig;
	cbConfig.SetBufferSize(sizeof(CBufferType));
	m_constantBuffer = m_pRender->CreateConstantBuffer(&cbConfig, 0);

	//create new rasterizer state
	RasterizerStateConfigDX11 rsStateConfig;
	rsStateConfig.GetDesc().ScissorEnable = true;
	m_rsStateID = m_pRender->CreateRasterizerState(&rsStateConfig);

	BuildShaders();
	BuildGeometry();
	BuildRenderTarget();

	return true;
}

void MSAA_Demo::BuildShaders()
{
	const std::wstring shaderfile = L"MSAA_Shader.hlsl";
	const std::wstring vs_5_0 = L"vs_5_0";
	const std::wstring gs_5_0 = L"gs_5_0";
	const std::wstring ps_5_0 = L"ps_5_0";

	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSMain", vs_5_0);
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSMain", ps_5_0);

	m_vsQuadID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSMainQuad", vs_5_0);
	m_psQuadID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSMainQuad", ps_5_0);

	m_psAAID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSMainAA", ps_5_0);
	m_gsID = m_pRender->LoadShader(ShaderType::GEOMETRY_SHADER, shaderfile, L"GSMain", gs_5_0);
}

void MSAA_Demo::BuildRenderTarget()
{
	DXGI_SAMPLE_DESC samp;
	samp.Count = 8;
	samp.Quality = 0;

	TextureRTConfig rtConfig;
	rtConfig.SetWidth(mClientWidth);
	rtConfig.SetHeight(mClientHeight);
	rtConfig.SetFormat(DF_R8G8B8A8_UNORM);
	rtConfig.SetSamp(8, 0);
	m_renderTargetTex = m_pRender->CreateTexture2D(&rtConfig, 0);

	TextureDSConfig dsConfig;
	dsConfig.SetWidth(mClientWidth);
	dsConfig.SetHeight(mClientHeight);
	dsConfig.SetFormat(DF_D24_UNORM_S8_UINT);
	dsConfig.SetSamp(8, 0);
	m_depthTargetTex = m_pRender->CreateTexture2D(&dsConfig, 0);

	Texture2dConfig texConfig2;
	texConfig2.SetWidth(mClientWidth);
	texConfig2.SetHeight(mClientHeight);
	texConfig2.SetFormat(DF_R8G8B8A8_UNORM);
	m_resolveTex = m_pRender->CreateTexture2D(&texConfig2, 0);

	SamplerStateConfigDX11 sampConfig;
	m_samplerID = m_pRender->CreateSamplerState(&sampConfig);
}

void MSAA_Demo::OnResize()
{
	Application::OnResize();
	SetupPipeline();
}

void MSAA_Demo::BuildGeometry()
{
	// create a box with GeometryDX11
	{
		m_pGeometry = GeometryPtr(new GeometryDX11());

		const i32 NumVertexOfBox = 8;
		// create the vertex element streams
		VertexElementDX11* pPositions = new VertexElementDX11(3, NumVertexOfBox);
		pPositions->m_SemanticName = VertexElementDX11::PositionSemantic;
		pPositions->m_uiSemanticIndex = 0;
		pPositions->m_Format = DXGI_FORMAT_R32G32B32_FLOAT;
		pPositions->m_uiInputSlot = 0;
		pPositions->m_uiAlignedByteOffset = 0;
		pPositions->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pPositions->m_uiInstanceDataStepRate = 0;

		VertexElementDX11* pColors = new VertexElementDX11(4, NumVertexOfBox);
		pColors->m_SemanticName = VertexElementDX11::ColorSemantic;
		pColors->m_uiSemanticIndex = 0;
		pColors->m_Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pColors->m_uiInputSlot = 0;
		pColors->m_uiAlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		pColors->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pColors->m_uiInstanceDataStepRate = 0;

		m_pGeometry->AddElement(pPositions);
		m_pGeometry->AddElement(pColors);


		*pPositions->Get3f(0) = Vector3f(-1.0f, -1.0f, -1.0f);
		*pPositions->Get3f(1) = Vector3f(-1.0f, +1.0f, -1.0f);
		*pPositions->Get3f(2) = Vector3f(+1.0f, +1.0f, -1.0f);
		*pPositions->Get3f(3) = Vector3f(+1.0f, -1.0f, -1.0f);
		*pPositions->Get3f(4) = Vector3f(-1.0f, -1.0f, +1.0f);
		*pPositions->Get3f(5) = Vector3f(-1.0f, +1.0f, +1.0f);
		*pPositions->Get3f(6) = Vector3f(+1.0f, +1.0f, +1.0f);
		*pPositions->Get3f(7) = Vector3f(+1.0f, -1.0f, +1.0f);

		*pColors->Get4f(0) = Colors::White;
		*pColors->Get4f(1) = Colors::Black;
		*pColors->Get4f(2) = Colors::Red;
		*pColors->Get4f(3) = Colors::Green;
		*pColors->Get4f(4) = Colors::Blue;
		*pColors->Get4f(5) = Colors::Yellow;
		*pColors->Get4f(6) = Colors::Cyan;
		*pColors->Get4f(7) = Colors::Magenta;

		m_pGeometry->AddFace(TriangleIndices(0, 1, 2));
		m_pGeometry->AddFace(TriangleIndices(0, 2, 3));

		m_pGeometry->AddFace(TriangleIndices(4, 6, 5));
		m_pGeometry->AddFace(TriangleIndices(4, 7, 6));

		m_pGeometry->AddFace(TriangleIndices(4, 5, 1));
		m_pGeometry->AddFace(TriangleIndices(4, 1, 0));

		m_pGeometry->AddFace(TriangleIndices(3, 2, 6));
		m_pGeometry->AddFace(TriangleIndices(3, 6, 7));

		m_pGeometry->AddFace(TriangleIndices(1, 5, 6));
		m_pGeometry->AddFace(TriangleIndices(1, 6, 2));

		m_pGeometry->AddFace(TriangleIndices(4, 0, 3));
		m_pGeometry->AddFace(TriangleIndices(4, 3, 7));

		m_pGeometry->LoadToBuffers();
	}

	// create a screen quad
	{
		m_pQuad = GeometryPtr(new GeometryDX11());

		const i32 NumVertexOfQuad = 4;
		// create the vertex element streams
		VertexElementDX11* pPositions = new VertexElementDX11(3, NumVertexOfQuad);
		pPositions->m_SemanticName = VertexElementDX11::PositionSemantic;
		pPositions->m_uiSemanticIndex = 0;
		pPositions->m_Format = DXGI_FORMAT_R32G32B32_FLOAT;
		pPositions->m_uiInputSlot = 0;
		pPositions->m_uiAlignedByteOffset = 0;
		pPositions->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pPositions->m_uiInstanceDataStepRate = 0;

		VertexElementDX11* pColors = new VertexElementDX11(4, NumVertexOfQuad);
		pColors->m_SemanticName = VertexElementDX11::ColorSemantic;
		pColors->m_uiSemanticIndex = 0;
		pColors->m_Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pColors->m_uiInputSlot = 0;
		pColors->m_uiAlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		pColors->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pColors->m_uiInstanceDataStepRate = 0;

		m_pQuad->AddElement(pPositions);
		m_pQuad->AddElement(pColors);

		*pPositions->Get3f(0) = Vector3f(-1.0f, +1.0f, 0.0f);
		*pPositions->Get3f(1) = Vector3f(+1.0f, +1.0f, 0.0f);
		*pPositions->Get3f(2) = Vector3f(-1.0f, -1.0f, 0.0f);
		*pPositions->Get3f(3) = Vector3f(+1.0f, -1.0f, 0.0f);
		*pColors->Get4f(0) = Colors::White;
		*pColors->Get4f(1) = Colors::White;
		*pColors->Get4f(2) = Colors::White;
		*pColors->Get4f(3) = Colors::White;

		m_pQuad->AddFace(TriangleIndices(0, 1, 2));
		m_pQuad->AddFace(TriangleIndices(1, 3, 2));

		m_pQuad->LoadToBuffers();
	}

	SetupPipeline();
}

void MSAA_Demo::SetupPipeline()
{
	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsID);
	vsState.ConstantBuffers.SetState(0, m_constantBuffer);
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psID);
	psState.ConstantBuffers.SetState(0, m_constantBuffer);
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
}

void MSAA_Demo::OnSpace()
{
	mAppPaused = !mAppPaused;
}

void MSAA_Demo::OnEnter()
{
	m_drawFrame = !m_drawFrame;
}