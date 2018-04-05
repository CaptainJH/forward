#include "dx11_Hieroglyph/ApplicationDX11.h"
#include "dx11_Hieroglyph/ResourceSystem/Buffer/BufferConfigDX11.h"
#include "TriangleIndices.h"
#include "FileLoader.h"
#include "dx11_Hieroglyph/ResourceSystem/Texture/Texture3dConfigDX11.h"
#include "dx11_Hieroglyph/ResourceSystem/StateObject/SamplerStateConfigDX11.h"

#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include "pystring.h"

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
	Vector4f screenParams;
	Vector4f cameraPos;
	Vector4f SDFParams;
	Vector4f SDFOrigin;
	Matrix4f matWorld;
	Matrix4f matViewProj;
	Matrix4f matViewProjInverse;
};

class SDFShadowDemo : public Application
{
public:
	SDFShadowDemo(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
		, m_vsID(-1)
		, m_psID(-1)
	{
		mMainWndCaption = L"SDFShadowDemo";
	}

	~SDFShadowDemo()
	{
		Log::Get().Close();
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	virtual void UpdateScene(f32 dt);
	virtual void DrawScene();
	virtual void OnSpace();

private:
	void BuildShaders();
	void LoadGeometry();
	void BuildQuad();
	void SetupPipeline();
	void LoadSDFBuffer();

	i32 m_vsID;
	i32 m_psID;

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;
	ResourcePtr m_constantBuffer;

	ResourcePtr m_SDFTex3D;

	std::vector<f32> m_sdf;
	Vector4f m_sdfParams;
	Vector4f m_sdfOrigin;
	std::vector<GeometryPtr> m_vGeoms;
	std::vector<Matrix4f> m_vMats;

	Vector4f m_cameraPos;

	GeometryPtr m_pQuad;
	i32 m_samplerID;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SDFShadowDemo theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void SDFShadowDemo::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	//m_worldMat = Matrix4f::RotationMatrixY(frames);
	m_worldMat = Matrix4f::Identity();

	Matrix4f viewRot = Matrix4f::RotationMatrixY(frames);
	// Build the view matrix.
	m_cameraPos = Vector4f(0.0f, 1.0f, -5.0f, 1.0f);
	m_cameraPos = viewRot * m_cameraPos;
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_viewMat = Matrix4f::LookAtLHMatrix(m_cameraPos.xyz(), target, up);
}

void SDFShadowDemo::DrawScene()
{
	m_pRender->pImmPipeline->ClearBuffers(Colors::LightSteelBlue);

	//for (auto i = 0; i < m_vGeoms.size(); ++i)
	//{
	//	auto pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
	//	auto pBuffer = (CBufferType*)pData.pData;
	//	pBuffer->mat = m_vMats[i] * m_worldMat * m_viewMat * m_projMat;
	//	m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);
	//	m_pRender->pImmPipeline->ApplyPipelineResources();

	//	m_vGeoms[i]->Execute(m_pRender->pImmPipeline);
	//}

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsID);
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	CBufferType buffer;
	buffer.matWorld = m_worldMat;
	buffer.matViewProj = m_viewMat * m_projMat;
	buffer.matViewProjInverse = (m_viewMat * m_projMat).Inverse();
	buffer.screenParams = Vector4f(static_cast<f32>(mClientWidth), static_cast<f32>(mClientHeight), 0.0f, 0.0f);
	buffer.SDFParams = m_sdfParams;
	buffer.SDFOrigin = m_sdfOrigin;
	buffer.cameraPos = m_cameraPos;
	m_pRender->pImmPipeline->UpdateBufferResource(m_constantBuffer, &buffer, sizeof(CBufferType));

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psID);
	psState.SamplerStates.SetState(0, m_pRender->GetSamplerState(m_samplerID).Get());
	psState.ShaderResources.SetState(0, m_SDFTex3D);
	psState.ConstantBuffers.SetState(0, m_constantBuffer);
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;

	m_pRender->pImmPipeline->ApplyPipelineResources();
	m_pQuad->Execute(m_pRender->pImmPipeline);

	m_pRender->Present(MainWnd(), 0);
}

bool SDFShadowDemo::Init()
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

	LoadSDFBuffer();
	BuildShaders();
	BuildQuad();
	LoadGeometry();

	SamplerStateConfigDX11 sampConfig;
	m_samplerID = m_pRender->CreateSamplerState(&sampConfig);

	return true;
}

void SDFShadowDemo::BuildShaders()
{
	const std::wstring shaderfile = L"SDFShaders.hlsl";
	const std::wstring VSMain = L"VSMainQuad";
	const std::wstring PSMain = L"PSMainQuad";
	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, VSMain, std::wstring(L"vs_5_0"));
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, PSMain, std::wstring(L"ps_5_0"));
}

void SDFShadowDemo::OnResize()
{
	Application::OnResize();
	SetupPipeline();
}

void SDFShadowDemo::LoadGeometry()
{
	// load mesh file
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;

	const std::wstring meshFileNameW = L"testScene.obj";
	const std::wstring meshFilePathW = FileSystem::getSingleton().GetModelsFolder();
	const std::string meshFileFullPath = TextHelper::ToAscii(meshFilePathW + meshFileNameW);

	pScene = imp.ReadFile(meshFileFullPath, aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_FlipWindingOrder);
	if (!pScene)
	{
		return;
	}

	auto count = pScene->mNumMeshes;
	if (count < 1)
		return;

	// load a scene into GeometryDX11
	for (auto id = 0U; id < count; ++id)
	{
		auto pMesh = pScene->mMeshes[id];
		auto pGeometry = GeometryPtr(new GeometryDX11());

		const i32 NumVertexOfBox = pMesh->mNumVertices;
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

		pGeometry->AddElement(pPositions);
		pGeometry->AddElement(pColors);

		for (auto i = 0; i < NumVertexOfBox; ++i)
		{
			aiVector3D pos = pMesh->mVertices[i];
			*pPositions->Get3f(i) = Vector3f(pos.x, pos.y, pos.z);
			*pColors->Get4f(i) = Colors::Green;
		}

		for (auto i = 0U; i < pMesh->mNumFaces; ++i)
		{
			aiFace face = pMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			pGeometry->AddFace(TriangleIndices(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
		}

		pGeometry->LoadToBuffers();
		m_vGeoms.push_back(pGeometry);
	}

	count = pScene->mRootNode->mNumChildren;
	for (auto i = 0U; i < count; ++i)
	{
		Matrix4f mat(true);
		aiNode* node = pScene->mRootNode->mChildren[i];
		for (auto j = 0; j < 4; ++j)
		{
			aiMatrix4x4 trans = node->mTransformation;
			auto ptr = trans[j];
			mat(j, 0) = ptr[0];
			mat(j, 1) = ptr[1];
			mat(j, 2) = ptr[2];
			mat(j, 3) = ptr[3];
		}

		mat = mat.Transpose();
		m_vMats.push_back(mat);
	}

	SetupPipeline();
}

void SDFShadowDemo::SetupPipeline()
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

void SDFShadowDemo::OnSpace()
{
	mAppPaused = !mAppPaused;
}

void SDFShadowDemo::BuildQuad()
{
	// create a screen quad
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

void SDFShadowDemo::LoadSDFBuffer()
{
	const std::wstring sdfFile = L"testScene.sdf";
	FileLoader loader;
	loader.Open(FileSystem::getSingleton().GetModelsFolder() + sdfFile);
	auto ptr = loader.GetDataPtr();
	std::string sdfText(ptr);
	std::vector<std::string> lines;
	
	pystring::splitlines(sdfText, lines);
	std::string line0 = lines[0];
	std::vector<std::string> line0Split;
	pystring::split(line0, line0Split, " ");
	m_sdfParams.x = strtof(line0Split[0].c_str(), 0);
	m_sdfParams.y = strtof(line0Split[1].c_str(), 0);
	m_sdfParams.z = strtof(line0Split[2].c_str(), 0);
	m_sdfParams.w = strtof(lines[2].c_str(), 0);
	
	std::string line1 = lines[1];
	std::vector<std::string> line1Split;
	pystring::split(line1, line1Split, " ");
	m_sdfOrigin.x = strtof(line1Split[0].c_str(), 0);
	m_sdfOrigin.y = strtof(line1Split[1].c_str(), 0);
	m_sdfOrigin.z = strtof(line1Split[2].c_str(), 0);

	const auto SDFCount = m_sdfParams.x * m_sdfParams.y * m_sdfParams.z;
	for (auto i = 0; i < SDFCount; ++i)
	{
		f32 value = strtof(lines[3 + i].c_str(), 0);
		m_sdf.push_back(value);
	}

	Texture3dConfig texConfig;
	texConfig.SetWidth(static_cast<u32>(m_sdfParams.x));
	texConfig.SetHeight(static_cast<u32>(m_sdfParams.y));
	texConfig.SetDepth(static_cast<u32>(m_sdfParams.z));
	texConfig.SetFormat(DF_R32_FLOAT);
	
	Subresource data;
	data.data = &m_sdf[0];
	data.rowPitch = static_cast<u32>(sizeof(f32) * m_sdfParams.x);
	data.slicePitch = static_cast<u32>(sizeof(f32) * m_sdfParams.x * m_sdfParams.y);

	m_SDFTex3D = m_pRender->CreateTexture3D(&texConfig, &data);
}