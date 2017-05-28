#include "ShadowMapApp.h"

void ShadowMapApp::UpdateScene(f32 dt)
{
	auto frames = (f32)mTimer.FrameCount() / 10000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);

	m_camMain.update(dt);
	m_camLight.update(dt);
	m_LiSP.update();
}

void ShadowMapApp::DrawScene()
{
	Matrix4f viewMat = m_camMain.updateViewMatrix();
	Matrix4f viewLight = m_camLight.updateViewMatrix();

	renderShadowTarget(viewLight);

	// Pass : draw the scene without any AA
	if (!m_drawShadowTarget)
	{
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_RenderTarget);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_DepthTarget);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

		CBufferType buffer;
		buffer.mat = m_worldMat * viewMat * m_camMain.getProjectionMatrix();
		if (m_useCSM)
		{
			buffer.matLight = m_worldMat * m_CSM.GetWorldToShadowSpace();
		}
		else
		{
			if (m_useLiSP)
				buffer.matLight = m_worldMat * m_LiSP.getMatrix();
			else
				buffer.matLight = m_worldMat * viewLight * m_camLight.getProjectionMatrix();
		}
		buffer.flags = Vector4f(m_usePCF ? 1.0f : 0.0f, m_useCSM ? 1.0f : 0.0f, m_usePCSS ? 1.0f : 0.0f, m_useVSM ? 1.0f : 0.0f);
		buffer.toCascadeOffsetX = m_CSM.GetToCascadeOffsetX();
		buffer.toCascadeOffsetY = m_CSM.GetToCascadeOffsetY();
		buffer.toCascadeScale = m_CSM.GetToCascadeScale();
		buffer.shadowMapPixelSize = 1.0f / mClientWidth;
		buffer.lightSize = 25.0f;
		m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsID);
		vsState.ConstantBuffers.SetState(0, m_constantBuffer);
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 gsState;
		gsState.ShaderProgram.SetState(-1);
		m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psID);
		psState.SamplerStates.SetState(0, m_pRender->GetSamplerState(m_samplerID).Get());
		psState.SamplerStates.SetState(1, m_pRender->GetSamplerState(m_pcfSamplerID).Get());
		psState.ShaderResources.SetState(0, m_shadowMapDepthTargetTex);
		psState.ShaderResources.SetState(1, m_CSMDepthTargetTex);
		if (m_useBlur)
			psState.ShaderResources.SetState(2, m_VSMBlurRenderTargetTex);
		else
			psState.ShaderResources.SetState(2, m_VSMRenderTargetTex);
		psState.ConstantBuffers.SetState(0, m_constantBuffer);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;

		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, 0);

		m_pRender->pImmPipeline->ApplyPipelineResources();
		m_pGeometry->Execute(m_pRender->pImmPipeline);

		buffer.mat = viewMat * m_camMain.getProjectionMatrix();
		if (m_useCSM)
		{
			buffer.matLight = m_CSM.GetWorldToShadowSpace();
		}
		else
		{
			if (m_useLiSP)
				buffer.matLight = m_LiSP.getMatrix();
			else
				buffer.matLight = viewLight * m_camLight.getProjectionMatrix();
		}
		buffer.flags = Vector4f(m_usePCF ? 1.0f : 0.0f, m_useCSM ? 1.0f : 0.0f, m_usePCSS ? 1.0f : 0.0f, m_useVSM ? 1.0f : 0.0f);
		buffer.toCascadeOffsetX = m_CSM.GetToCascadeOffsetX();
		buffer.toCascadeOffsetY = m_CSM.GetToCascadeOffsetY();
		buffer.toCascadeScale = m_CSM.GetToCascadeScale();
		buffer.shadowMapPixelSize = 1.0f / mClientWidth;
		buffer.lightSize = 25.0f;
		m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));
		m_pFloor->Execute(m_pRender->pImmPipeline);

		// clear shader resource view
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState.ShaderResources.SetState(0, nullptr);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState.ShaderResources.SetState(1, nullptr);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState.ShaderResources.SetState(2, nullptr);
		m_pRender->pImmPipeline->ApplyPipelineResources();
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
	Vector3f pos = Vector3f(0.0f, 0.0f, -9.0f);
	Vector3f target; target.MakeZero();
	target.y = 1.0f;
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_camMain.setViewMatrix(Matrix4f::LookAtLHMatrix(pos, target, up));
	m_camLight.setViewMatrix(Matrix4f::LookAtLHMatrix(Vector3f(120.0f, 70.0f, 0.0f), target, up));
	// Build the projection matrix
	m_camMain.setProjectionParams(0.5f * Pi, AspectRatio(), 0.01f, 50.0f);
	m_camLight.setProjectionParams(0.25f * Pi, AspectRatio(), 1.0f, 150.0f);

	BufferConfigDX11 cbConfig;
	cbConfig.SetDefaultConstantBuffer(sizeof(CBufferType), true);
	m_constantBuffer = m_pRender->CreateConstantBuffer(&cbConfig, 0);

	BuildShaders();
	BuildGeometry();
	BuildRenderTarget();

	m_LiSP.addSceneObjects(m_pGeometry.get());
	m_LiSP.addSceneObjects(m_pFloor.get());
	m_LiSP.linkWorldMatrixOfObject(m_pGeometry.get(), &m_worldMat);

	return true;
}

void ShadowMapApp::BuildShaders()
{
	const std::wstring shaderfile = L"ShadowMapShaders.hlsl";
	const std::wstring msaaShaderfile = L"MSAA_Shader.hlsl";
	const std::wstring vs_5_0 = L"vs_5_0";
	const std::wstring gs_5_0 = L"gs_5_0";
	const std::wstring ps_5_0 = L"ps_5_0";

	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSMain", vs_5_0);
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSMain", ps_5_0);

	m_vsShadowTargetID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSShadowTargetMain", vs_5_0);
	m_psShadowTargetID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSShadowTargetMain", ps_5_0);

	m_vsCSMID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSCSMTargetMain", vs_5_0);
	m_gsCSMID = m_pRender->LoadShader(ShaderType::GEOMETRY_SHADER, shaderfile, L"GSCSMTargetMain", gs_5_0);

	m_psVSMDepthGenID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSVSMDepthGenMain", ps_5_0);

	m_vsQuadID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, msaaShaderfile, L"VSMainQuad", vs_5_0);
	m_psQuadBlurID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSQuadBlurMain", ps_5_0);
}

void ShadowMapApp::BuildRenderTarget()
{
	u32 shadowTargetWidth = static_cast<u32>(mClientWidth * 2);
	u32 shadowTargetHeight = static_cast<u32>(mClientHeight * 2);

	m_blurCoefW = 1.0f / (m_blurCoef * shadowTargetWidth);
	m_blurCoefH = 1.0f / (m_blurCoef * shadowTargetHeight);

	Texture2dConfigDX11 texConfig;
	texConfig.SetColorBuffer(shadowTargetWidth, shadowTargetHeight);
	texConfig.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	m_shadowMapRenderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	/// create depth buffer texture for spot light
	texConfig.SetDepthBuffer(shadowTargetWidth, shadowTargetHeight);
	texConfig.MakeDepthStencilAndShaderResource();
	m_shadowMapDepthTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	/// create render target buffer for CSM
	texConfig.SetColorBuffer(shadowTargetWidth, shadowTargetWidth);
	texConfig.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET);
	texConfig.SetArraySize(m_CSM.m_iTotalCascades);
	m_CSMRenderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	/// create depth buffer texture for CSM
	texConfig.SetDepthBuffer(shadowTargetWidth, shadowTargetWidth);
	texConfig.MakeDepthStencilAndShaderResource();
	texConfig.SetArraySize(m_CSM.m_iTotalCascades);
	m_CSMDepthTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	m_CSM.Init(shadowTargetWidth);

	/// create render target buffer VSM
	texConfig.SetColorBuffer(shadowTargetWidth, shadowTargetHeight);
	texConfig.SetFormat(DXGI_FORMAT_R32G32_FLOAT);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	m_VSMRenderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	/// create render target buffer for blurring
	texConfig.SetColorBuffer(static_cast<u32>(shadowTargetWidth * m_blurCoef), static_cast<u32>(shadowTargetHeight * m_blurCoef));
	texConfig.SetFormat(DXGI_FORMAT_R32G32_FLOAT);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	m_VSMBlurRenderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	m_shadowMapViewportID = m_pRender->CreateViewPort(shadowTargetWidth, shadowTargetHeight);
	m_CSMViewportID = m_pRender->CreateViewPort(shadowTargetWidth, shadowTargetWidth);

	u32 width = static_cast<u32>(shadowTargetWidth * m_blurCoef);
	u32 height = static_cast<u32>(shadowTargetHeight * m_blurCoef);
	m_blurViewportID = m_pRender->CreateViewPort(width, height);

	SamplerStateConfigDX11 sampConfig;
	m_samplerID = m_pRender->CreateSamplerState(&sampConfig);

	// Create the PCF sampler state
	sampConfig.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampConfig.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_pcfSamplerID = m_pRender->CreateSamplerState(&sampConfig);
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

	// create the floor: shadow receiver
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

		*pPositions->Get3f(0) = Vector3f(-10.0f, -1.0f, +10.0f);
		*pPositions->Get3f(1) = Vector3f(+10.0f, -1.0f, +10.0f);
		*pPositions->Get3f(2) = Vector3f(-10.0f, -1.0f, -10.0f);
		*pPositions->Get3f(3) = Vector3f(+10.0f, -1.0f, -10.0f);
		*pColors->Get4f(0) = Colors::White;
		*pColors->Get4f(1) = Colors::White;
		*pColors->Get4f(2) = Colors::White;
		*pColors->Get4f(3) = Colors::White;

		m_pFloor->AddFace(TriangleIndices(0, 1, 2));
		m_pFloor->AddFace(TriangleIndices(1, 3, 2));

		m_pFloor->LoadToBuffers();
	}

	// create a screen quad to draw blurring shadow texture
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

void ShadowMapApp::SetupPipeline()
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

void ShadowMapApp::OnSpace()
{
	mAppPaused = !mAppPaused;
}

void ShadowMapApp::OnEnter()
{
	m_drawShadowTarget = !m_drawShadowTarget;
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

	case 'p':
		m_usePCF = !m_usePCF;
		break;

	case 'c':
		m_useCSM = !m_useCSM;
		break;

	case 'l':
		m_useLiSP = !m_useLiSP;
		break;

	case 'z':
		m_usePCSS = !m_usePCSS;
		break;

	case 'v':
		m_useVSM = !m_useVSM;
		break;

	case 'b':
		m_useBlur = !m_useBlur;
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

void ShadowMapApp::renderSpotLightShadowMap(const Matrix4f& ViewLight)
{
	if (!m_drawShadowTarget)
	{
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, m_shadowMapViewportID);

		// setup render target
		if (m_useVSM)
			m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_VSMRenderTargetTex);
		else
			m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_shadowMapRenderTargetTex);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_shadowMapDepthTargetTex);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);
	}
	else
	{
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, 0);

		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_RenderTarget);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_DepthTarget);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Black);
	}

	CBufferType buffer;
	if (m_useLiSP)
		buffer.matLight = m_worldMat * m_LiSP.getMatrix();
	else
		buffer.matLight = m_worldMat * ViewLight * m_camLight.getProjectionMatrix();
	m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsShadowTargetID);
	vsState.ConstantBuffers.SetState(0, m_constantBuffer);
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 gsState;
	gsState.ShaderProgram.SetState(-1);
	m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

	ShaderStageStateDX11 psState;
	if (m_useVSM)
		psState.ShaderProgram.SetState(m_psVSMDepthGenID);
	else
		psState.ShaderProgram.SetState(m_psShadowTargetID);
	psState.ConstantBuffers.SetState(0, m_constantBuffer);
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
	m_pRender->pImmPipeline->ApplyPipelineResources();

	m_pGeometry->Execute(m_pRender->pImmPipeline);

	if (m_useLiSP)
		buffer.matLight = m_LiSP.getMatrix();
	else
		buffer.matLight = ViewLight * m_camLight.getProjectionMatrix();
	m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));
	m_pFloor->Execute(m_pRender->pImmPipeline);
}

void ShadowMapApp::renderCSMShadowMap()
{
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(m_CSM.m_iTotalCascades);
	for (auto i = 0; i < m_CSM.m_iTotalCascades; ++i)
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(i, m_CSMViewportID);

	// setup render target
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_CSMRenderTargetTex);
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_CSMDepthTargetTex);
	m_pRender->pImmPipeline->ApplyRenderTargets();
	m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

	auto lightDir = m_camLight.getWorldLookingDir();
	m_CSM.Update(lightDir);

	CBufferType buffer;
	for (auto i = 0; i < m_CSM.m_iTotalCascades; ++i)
		buffer.matCSM[i] = m_worldMat * m_CSM.GetWorldToCascadeProj(i);
	m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsCSMID);
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 gsState;
	gsState.ShaderProgram.SetState(m_gsCSMID);
	gsState.ConstantBuffers.SetState(0, m_constantBuffer);
	m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psShadowTargetID);
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
	m_pRender->pImmPipeline->ApplyPipelineResources();

	m_pGeometry->Execute(m_pRender->pImmPipeline);

	for (auto i = 0; i < m_CSM.m_iTotalCascades; ++i)
		buffer.matCSM[i] = m_CSM.GetWorldToCascadeProj(i);
	m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));
	m_pFloor->Execute(m_pRender->pImmPipeline);
}

void ShadowMapApp::renderShadowTarget(const Matrix4f& ViewLight)
{
	renderSpotLightShadowMap(ViewLight);
	if (m_useBlur)
		blurShadowRenderTarget(m_VSMBlurRenderTargetTex);
	renderCSMShadowMap();
}

void ShadowMapApp::blurShadowRenderTarget(ResourcePtr ptr)
{
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, m_blurViewportID);

	// setup render target
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_VSMBlurRenderTargetTex);
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(0); // use null here!
	m_pRender->pImmPipeline->ApplyRenderTargets();
	m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

	CBufferType buffer;

	///blur horizontally
	{
		buffer.blurFactorX = m_blurCoefW;
		buffer.blurFactorY = 0.0f;
		m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsQuadID);
		vsState.ConstantBuffers.SetState(0, m_constantBuffer);
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 gsState;
		gsState.ShaderProgram.SetState(-1);
		m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psQuadBlurID);
		psState.SamplerStates.SetState(0, m_pRender->GetSamplerState(m_samplerID).Get());
		psState.ConstantBuffers.SetState(0, m_constantBuffer);
		psState.ShaderResources.SetState(2, m_VSMRenderTargetTex);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
		m_pRender->pImmPipeline->ApplyPipelineResources();

		m_pQuad->Execute(m_pRender->pImmPipeline);
	}

	///blur vertically
	{
		buffer.blurFactorX = 0.0f;
		buffer.blurFactorY = m_blurCoefH;
		m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsQuadID);
		vsState.ConstantBuffers.SetState(0, m_constantBuffer);
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 gsState;
		gsState.ShaderProgram.SetState(-1);
		m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psQuadBlurID);
		psState.SamplerStates.SetState(0, m_pRender->GetSamplerState(m_samplerID).Get());
		psState.ConstantBuffers.SetState(0, m_constantBuffer);
		psState.ShaderResources.SetState(2, m_VSMRenderTargetTex);
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
		m_pRender->pImmPipeline->ApplyPipelineResources();

		m_pQuad->Execute(m_pRender->pImmPipeline);
	}

	// clear shader resource view
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState.ShaderResources.SetState(2, nullptr);
	m_pRender->pImmPipeline->ApplyPipelineResources();
}