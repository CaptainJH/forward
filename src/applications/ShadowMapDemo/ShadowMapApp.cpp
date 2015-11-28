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
	ResourceDX11* resource = m_pRender->GetResourceByIndex(m_constantBuffer->m_iResource);
	Matrix4f viewMat = m_camMain.updateViewMatrix();
	Matrix4f viewLight = m_camLight.updateViewMatrix();

	renderShadowTarget(viewLight);

	// Pass : draw the scene without any AA
	if(!m_drawShadowTarget)
	{
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetViews.SetState(0, m_RenderTarget->m_iResourceRTV);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetViews.SetState(m_DepthTarget->m_iResourceDSV);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

		auto pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
		auto pBuffer = (CBufferType*)pData.pData;
		pBuffer->mat = m_worldMat * viewMat * m_camMain.getProjectionMatrix();
		if (m_useCSM)
		{
			pBuffer->matLight = m_worldMat * m_CSM.GetWorldToShadowSpace();
		}
		else
		{
			if (m_useLiSP)
				pBuffer->matLight = m_LiSP.getMatrix();
			else
				pBuffer->matLight = m_worldMat * viewLight * m_camLight.getProjectionMatrix();
		}
		pBuffer->flags = Vector4f(m_usePCF ? 1.0f : 0.0f, m_useCSM ? 1.0f : 0.0f, m_usePCSS ? 1.0f : 0.0f, 0.0f);
		pBuffer->toCascadeOffsetX = m_CSM.GetToCascadeOffsetX();
		pBuffer->toCascadeOffsetY = m_CSM.GetToCascadeOffsetY();
		pBuffer->toCascadeScale = m_CSM.GetToCascadeScale();
		pBuffer->shadowMapPixelSize = 1.0f / mClientWidth;
		pBuffer->lightSize = 25.0f;
		m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);

		ShaderStageStateDX11 vsState;
		vsState.ShaderProgram.SetState(m_vsID);
		vsState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
		m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

		ShaderStageStateDX11 gsState;
		gsState.ShaderProgram.SetState(-1);
		m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

		ShaderStageStateDX11 psState;
		psState.ShaderProgram.SetState(m_psID);
		psState.SamplerStates.SetState(0, m_pRender->GetSamplerState(m_samplerID).Get());
		psState.SamplerStates.SetState(1, m_pRender->GetSamplerState(m_pcfSamplerID).Get());
		psState.ShaderResourceViews.SetState(0, m_pRender->GetShaderResourceViewByIndex(m_shadowMapDepthTargetTex->m_iResourceSRV).GetSRV());
		psState.ShaderResourceViews.SetState(1, m_pRender->GetShaderResourceViewByIndex(m_CSMDepthTargetTex->m_iResourceSRV).GetSRV());
		psState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;

		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, 0);

		m_pRender->pImmPipeline->ApplyPipelineResources();
		m_pGeometry->Execute(m_pRender->pImmPipeline);

		pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
		pBuffer = (CBufferType*)pData.pData;
		pBuffer->mat = viewMat * m_camMain.getProjectionMatrix();
		if (m_useCSM)
		{
			pBuffer->matLight = m_CSM.GetWorldToShadowSpace();
		}
		else
		{
			if (m_useLiSP)
				pBuffer->matLight = m_LiSP.getMatrix();
			else
				pBuffer->matLight = viewLight * m_camLight.getProjectionMatrix();
		}
		pBuffer->flags = Vector4f(m_usePCF ? 1.0f : 0.0f, m_useCSM ? 1.0f : 0.0f, m_usePCSS ? 1.0f : 0.0f, 0.0f);
		pBuffer->toCascadeOffsetX = m_CSM.GetToCascadeOffsetX();
		pBuffer->toCascadeOffsetY = m_CSM.GetToCascadeOffsetY();
		pBuffer->toCascadeScale = m_CSM.GetToCascadeScale();
		pBuffer->shadowMapPixelSize = 1.0f / mClientWidth;
		pBuffer->lightSize = 25.0f;
		m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);
		m_pFloor->Execute(m_pRender->pImmPipeline);

		// clear shader resource view
		m_pRender->pImmPipeline->PixelShaderStage.DesiredState.ShaderResourceViews.SetState(0, nullptr);
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
	m_camLight.setViewMatrix(Matrix4f::LookAtLHMatrix(Vector3f(2.0f, 20.0f, 2.0f), target, up));
	// Build the projection matrix
	m_camMain.setProjectionParams(0.5f * Pi, AspectRatio(), 0.01f, 50.0f);
	m_camLight.setProjectionParams(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);

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
	const std::wstring vs_5_0 = L"vs_5_0";
	const std::wstring gs_5_0 = L"gs_5_0";
	const std::wstring ps_5_0 = L"ps_5_0";

	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSMain", vs_5_0);
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSMain", ps_5_0);

	m_vsShadowTargetID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSShadowTargetMain", vs_5_0);
	m_psShadowTargetID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, L"PSShadowTargetMain", ps_5_0);

	m_vsCSMID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, L"VSCSMTargetMain", vs_5_0);
	m_gsCSMID = m_pRender->LoadShader(ShaderType::GEOMETRY_SHADER, shaderfile, L"GSCSMTargetMain", gs_5_0);
}

void ShadowMapApp::BuildRenderTarget()
{
	u32 shadowTargetWidth = static_cast<u32>(mClientWidth * 2);
	u32 shadowTargetHeight = static_cast<u32>(mClientHeight * 2);

	Texture2dConfigDX11 texConfig;
	texConfig.SetColorBuffer(shadowTargetWidth, shadowTargetHeight);
	texConfig.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	m_shadowMapRenderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	/// create depth buffer texture for spot light
	texConfig.SetDepthBuffer(shadowTargetWidth, shadowTargetHeight);
	texConfig.SetFormat(DXGI_FORMAT_R32_TYPELESS);
	texConfig.SetBindFlags(D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	ShaderResourceViewConfigDX11 srvConfig;
	srvConfig.SetFormat(DXGI_FORMAT_R32_FLOAT);
	D3D11_TEX2D_SRV texSrv;
	texSrv.MipLevels = 1;
	texSrv.MostDetailedMip = 0;
	srvConfig.SetTexture2D(texSrv);
	DepthStencilViewConfigDX11 dsvConfig;
	dsvConfig.SetFlags(0);
	dsvConfig.SetFormat(DXGI_FORMAT_D32_FLOAT);
	D3D11_TEX2D_DSV texDsv;
	texDsv.MipSlice = 0;
	dsvConfig.SetTexture2D(texDsv);
	m_shadowMapDepthTargetTex = m_pRender->CreateTexture2D(&texConfig, 0, &srvConfig, 0, 0, &dsvConfig);

	/// create render target buffer for CSM
	texConfig.SetColorBuffer(shadowTargetWidth, shadowTargetWidth);
	texConfig.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	texConfig.SetBindFlags(D3D11_BIND_RENDER_TARGET);
	texConfig.SetArraySize(m_CSM.m_iTotalCascades);
	m_CSMRenderTargetTex = m_pRender->CreateTexture2D(&texConfig, 0);

	/// create depth buffer texture for CSM
	texConfig.SetDepthBuffer(shadowTargetWidth, shadowTargetWidth);
	texConfig.SetFormat(DXGI_FORMAT_R32_TYPELESS);
	texConfig.SetBindFlags(D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	texConfig.SetArraySize(m_CSM.m_iTotalCascades);
	texConfig.SetMiscFlags(0);
	D3D11_TEX2D_ARRAY_SRV csmSrv;
	csmSrv.ArraySize = m_CSM.m_iTotalCascades;
	csmSrv.FirstArraySlice = 0;
	csmSrv.MipLevels = 1;
	csmSrv.MostDetailedMip = 0;
	srvConfig.SetTexture2DArray(csmSrv);
	D3D11_TEX2D_ARRAY_DSV csmDsv;
	csmDsv.ArraySize = m_CSM.m_iTotalCascades;
	csmDsv.FirstArraySlice = 0;
	csmDsv.MipSlice = 0;
	dsvConfig.SetTexture2DArray(csmDsv);
	m_CSMDepthTargetTex = m_pRender->CreateTexture2D(&texConfig, 0, &srvConfig, 0, 0, &dsvConfig);

	m_CSM.Init(shadowTargetWidth);

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<f32>(shadowTargetWidth);
	viewport.Height = static_cast<f32>(shadowTargetHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_shadowMapViewportID = m_pRender->CreateViewPort(viewport);

	viewport.Width = static_cast<f32>(shadowTargetWidth);
	viewport.Height = static_cast<f32>(shadowTargetWidth);
	m_CSMViewportID = m_pRender->CreateViewPort(viewport);

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

		*pPositions->Get3f(0) = Vector3f(-5.0f, -3.0f, +5.0f);
		*pPositions->Get3f(1) = Vector3f(+5.0f, -3.0f, +5.0f);
		*pPositions->Get3f(2) = Vector3f(-5.0f, -3.0f, -5.0f);
		*pPositions->Get3f(3) = Vector3f(+5.0f, -3.0f, -5.0f);
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
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetViews.SetState(0, m_shadowMapRenderTargetTex->m_iResourceRTV);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetViews.SetState(m_shadowMapDepthTargetTex->m_iResourceDSV);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);
	}
	else
	{
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, 0);

		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetViews.SetState(0, m_RenderTarget->m_iResourceRTV);
		m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetViews.SetState(m_DepthTarget->m_iResourceDSV);
		m_pRender->pImmPipeline->ApplyRenderTargets();
		m_pRender->pImmPipeline->ClearBuffers(Colors::Black);
	}

	ResourceDX11* resource = m_pRender->GetResourceByIndex(m_constantBuffer->m_iResource);

	auto pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
	auto pBuffer = (CBufferType*)pData.pData;
	if (m_useLiSP)
		pBuffer->matLight = m_worldMat * m_LiSP.getMatrix();
	else
		pBuffer->matLight = m_worldMat * ViewLight * m_camLight.getProjectionMatrix();
	m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsShadowTargetID);
	vsState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 gsState;
	gsState.ShaderProgram.SetState(-1);
	m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psShadowTargetID);
	psState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
	m_pRender->pImmPipeline->ApplyPipelineResources();

	m_pGeometry->Execute(m_pRender->pImmPipeline);

	pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
	pBuffer = (CBufferType*)pData.pData;
	if (m_useLiSP)
		pBuffer->matLight = m_LiSP.getMatrix();
	else
		pBuffer->matLight = ViewLight * m_camLight.getProjectionMatrix();
	m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);
	m_pFloor->Execute(m_pRender->pImmPipeline);
}

void ShadowMapApp::renderCSMShadowMap()
{
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(m_CSM.m_iTotalCascades);
	for (auto i = 0; i < m_CSM.m_iTotalCascades; ++i)
		m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(i, m_CSMViewportID);

	// setup render target
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetViews.SetState(0, m_CSMRenderTargetTex->m_iResourceRTV);
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetViews.SetState(m_CSMDepthTargetTex->m_iResourceDSV);
	m_pRender->pImmPipeline->ApplyRenderTargets();
	m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);

	auto lightDir = m_camLight.getWorldLookingDir();
	m_CSM.Update(lightDir);

	ResourceDX11* resource = m_pRender->GetResourceByIndex(m_constantBuffer->m_iResource);

	auto pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
	auto pBuffer = (CBufferType*)pData.pData;
	for (auto i = 0; i < m_CSM.m_iTotalCascades; ++i)
		pBuffer->matCSM[i] = m_worldMat * m_CSM.GetWorldToCascadeProj(i);
	m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsCSMID);
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 gsState;
	gsState.ShaderProgram.SetState(m_gsCSMID);
	gsState.ConstantBuffers.SetState(0, (ID3D11Buffer*)resource->GetResource());
	m_pRender->pImmPipeline->GeometryShaderStage.DesiredState = gsState;

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psShadowTargetID);
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
	m_pRender->pImmPipeline->ApplyPipelineResources();

	m_pGeometry->Execute(m_pRender->pImmPipeline);

	pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
	pBuffer = (CBufferType*)pData.pData;
	for (auto i = 0; i < m_CSM.m_iTotalCascades; ++i)
		pBuffer->matCSM[i] = m_CSM.GetWorldToCascadeProj(i);
	m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);
	m_pFloor->Execute(m_pRender->pImmPipeline);
}

void ShadowMapApp::renderShadowTarget(const Matrix4f& ViewLight)
{
	renderSpotLightShadowMap(ViewLight);
	renderCSMShadowMap();
}