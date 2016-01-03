#include "ApplicationDX11.h"
#include "BufferConfigDX11.h"
#include "TriangleIndices.h"

#include "GeometryLoader.h"


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
};

class AssimpDemo : public Application
{
public:
	AssimpDemo(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
		, m_vsID(-1)
		, m_psID(-1)
	{
		mMainWndCaption = L"AssimpDemo";
	}

	~AssimpDemo()
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
	void SetupPipeline();

	i32 m_vsID;
	i32 m_psID;

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;
	ResourcePtr m_constantBuffer;

	std::vector<GeometryPtr> m_vGeoms;
	std::vector<Matrix4f> m_vMats;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	AssimpDemo theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void AssimpDemo::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames);
}

void AssimpDemo::DrawScene()
{
	m_pRender->pImmPipeline->ClearBuffers(Colors::LightSteelBlue);

	for (auto i = 0; i < m_vGeoms.size(); ++i)
	{
		auto pData = m_pRender->pImmPipeline->MapResource(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0);
		auto pBuffer = (CBufferType*)pData.pData;
		pBuffer->mat = m_vMats[i] * m_worldMat * m_viewMat * m_projMat;
		m_pRender->pImmPipeline->UnMapResource(m_constantBuffer, 0);
		m_pRender->pImmPipeline->ApplyPipelineResources();

		m_vGeoms[i]->Execute(m_pRender->pImmPipeline);
	}

	m_pRender->Present(MainWnd(), 0);
}

bool AssimpDemo::Init()
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

	BufferConfigDX11 cbConfig;
	cbConfig.SetDefaultConstantBuffer(sizeof(CBufferType), true);
	m_constantBuffer = m_pRender->CreateConstantBuffer(&cbConfig, 0);

	BuildShaders();
	LoadGeometry();

	return true;
}

void AssimpDemo::BuildShaders()
{
	const std::wstring shaderfile = L"BasicShader.hlsl";
	const std::wstring VSMain = L"VSMain";
	const std::wstring PSMain = L"PSMain";
	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, VSMain, std::wstring(L"vs_5_0"));
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, PSMain, std::wstring(L"ps_5_0"));
}

void AssimpDemo::OnResize()
{
	Application::OnResize();
	SetupPipeline();
}

void AssimpDemo::LoadGeometry()
{
	const std::wstring meshFileNameW = L"simpleScene.dae";
	//const std::wstring meshFilePathW = FileSystem::getSingleton().GetModelsFolder();
	//const std::string meshFileFullPath = TextHelper::ToAscii(meshFilePathW + meshFileNameW);

	GeometryLoader::loadMeshFileDX(meshFileNameW, m_vGeoms, m_vMats);

	SetupPipeline();
}

void AssimpDemo::SetupPipeline()
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

void AssimpDemo::OnSpace()
{
	mAppPaused = !mAppPaused;
}