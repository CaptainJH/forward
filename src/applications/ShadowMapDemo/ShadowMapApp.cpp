#include "ShadowMapApp.h"

void ShadowMapApp::UpdateScene(f32 dt)
{
	auto frames = (f32)mTimer.FrameCount() / 10000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);

	m_camMain.update(dt);
}

void ShadowMapApp::DrawScene()
{
	m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

	ResourceDX11* resource = m_pRender->GetResourceByIndex(m_constantBuffer->m_iResource);

	Matrix4f viewMat = m_camMain.upViewMatrix();

	// Pass 1 : draw a cube without any AA
	{
		auto pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
		auto pBuffer = (CBufferType*)pData.pData;
		pBuffer->mat = m_worldMat * viewMat * m_camMain.getProjectionMatrix();
		m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsID);
		vsState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psID);
		psState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;

		m_pRender->pImmPipeline->ApplyPipelineResources();
		m_pGeometry->Execute(m_pRender->pImmPipeline);

		pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
		pBuffer = (CBufferType*)pData.pData;
		pBuffer->mat = viewMat * m_camMain.getProjectionMatrix();
		m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);
		m_pFloor->Execute(m_pRender->pImmPipeline);
	}

	// draw the floor
	{

	}

	m_pRender->Present(MainWnd(), 0);
}

bool ShadowMapApp::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	// Init the world matrix
	m_worldMat = Matrix4f::Identity();
	// Build the view matrix.
	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	target.y = 1.0f;
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_camMain.setViewMatrix(Matrix4f::LookAtLHMatrix(pos, target, up));
	// Build the projection matrix
	m_camMain.setProjectionParams(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);

	BufferConfigDX11 cbConfig;
	cbConfig.SetDefaultConstantBuffer(sizeof(CBufferType), true);
	m_constantBuffer = m_pRender->CreateConstantBuffer(&cbConfig, 0);

	//create new rasterizer state
	RasterizerStateConfigDX11 rsStateConfig;
	rsStateConfig.ScissorEnable = true;
	m_rsStateID = m_pRender->CreateRasterizerState(&rsStateConfig);

	BuildShaders();
	BuildGeometry();
	BuildRenderTarget();

	POINT mousePos;
	///TODO: windows API!
	GetCursorPos(&mousePos);
	m_preMouseX = mousePos.x;
	m_preMouseY = mousePos.y;

	return true;
}

void ShadowMapApp::BuildShaders()
{
	const std::wstring shaderfile = L"ShadowMapShaders.hlsl";
	const std::wstring vs_5_0 = L"vs_5_0";
	const std::wstring gs_5_0 = L"gs_5_0";
	const std::wstring ps_5_0 = L"ps_5_0";

	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSMain", vs_5_0);
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSMain", ps_5_0);
}

void ShadowMapApp::BuildRenderTarget()
{
	DXGI_SAMPLE_DESC samp;
	samp.Count = 8;
	samp.Quality = 0;

	Texture2dConfigDX11 texConfig;
	texConfig.SetColorBuffer(mClientWidth, mClientHeight);
	texConfig.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	texConfig.SetSampleDesc(samp);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET);
	m_renderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	texConfig.SetDepthBuffer(mClientWidth, mClientHeight);
	texConfig.SetFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	texConfig.SetSampleDesc(samp);
	m_depthTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	texConfig.SetColorBuffer(mClientWidth, mClientHeight);
	texConfig.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_resolveTex = m_pRender->CreateTexture2D(&texConfig, 0);

	SamplerStateConfigDX11 sampConfig;
	m_samplerID = m_pRender->CreateSamplerState(&sampConfig);
}

void ShadowMapApp::OnResize()
{
	Application::OnResize();
	SetupPipeline();
}

void ShadowMapApp::BuildGeometry()
{
	// create a box with GeometryDX11 : shadow caster
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

	// create a floor: shaow receiver
	{
		m_pFloor = GeometryPtr(new GeometryDX11());

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

		m_pFloor->AddElement(pPositions);
		m_pFloor->AddElement(pColors);

		*pPositions->Get3f(0) = Vector3f(-10.0f, -3.0f, +10.0f);
		*pPositions->Get3f(1) = Vector3f(+10.0f, -3.0f, +10.0f);
		*pPositions->Get3f(2) = Vector3f(-10.0f, -3.0f, -10.0f);
		*pPositions->Get3f(3) = Vector3f(+10.0f, -3.0f, -10.0f);
		*pColors->Get4f(0) = Colors::White;
		*pColors->Get4f(1) = Colors::White;
		*pColors->Get4f(2) = Colors::White;
		*pColors->Get4f(3) = Colors::White;

		m_pFloor->AddFace(TriangleIndices(0, 1, 2));
		m_pFloor->AddFace(TriangleIndices(1, 3, 2));

		m_pFloor->LoadToBuffers();
	}

	SetupPipeline();
}

void ShadowMapApp::SetupPipeline()
{
	auto resource = m_pRender->GetResourceByIndex(m_constantBuffer->m_iResource);

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsID);
	vsState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psID);
	psState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
}

void ShadowMapApp::OnSpace()
{
	mAppPaused = !mAppPaused;
}

void ShadowMapApp::OnEnter()
{
	m_drawFrame = !m_drawFrame;
}

void ShadowMapApp::OnChar(i8 key)
{
	switch (key)
	{
	case 'w':
	case 'a':
	case 's':
	case 'd':
		m_camMain.updateWithKeyboardInput(key);
		break;
	}
}

void ShadowMapApp::OnMouseMove(WPARAM btnState, i32 x, i32 y)
{
	auto btn = btnState & MK_LBUTTON;
	if (btn)
	{
		if (x != m_preMouseX || y != m_preMouseY)
		{
			m_camMain.updateWithMouseInput(x - m_preMouseX, y - m_preMouseY);
			m_preMouseX = x;
			m_preMouseY = y;
		}
	}
}

void ShadowMapApp::OnMouseDown(WPARAM btnState, i32 x, i32 y)
{
	auto btn = btnState & MK_LBUTTON;
	if (btn)
	{
		m_preMouseX = x;
		m_preMouseY = y;
	}
}