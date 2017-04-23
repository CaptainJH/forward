#include "ApplicationDX11.h"
#include "ResourceSystem\Buffer\BufferConfigDX11.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
};

class HelloWorld : public Application
{
public:
	HelloWorld(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
		, m_vsID(-1)
		, m_psID(-1)
	{
		mMainWndCaption = L"Hello World!";
	}

	~HelloWorld()
	{
		Log::Get().Close();
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	virtual void UpdateScene(f32 dt);
	virtual void DrawScene();

private:
	void BuildShaders();
	void BuildGeometry();
	void SetupPipeline();

	i32 m_vsID;
	i32 m_psID;

	ResourcePtr m_pVertexBuffer;
	ResourcePtr m_pIndexBuffer;
	i32 m_VertexLayout;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HelloWorld theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void HelloWorld::UpdateScene(f32 /*dt*/)
{
}

void HelloWorld::DrawScene()
{
	m_pRender->pImmPipeline->ClearBuffers(Colors::LightSteelBlue);

	m_pRender->pImmPipeline->ApplyInputResources();
	m_pRender->pImmPipeline->ApplyPipelineResources();
	m_pRender->pImmPipeline->Draw(4, 0);

	m_pRender->Present(MainWnd(), 0);
}

bool HelloWorld::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	BuildShaders();
	BuildGeometry();

	return true;
}

void HelloWorld::BuildShaders()
{
	const std::wstring shaderfile = L"BasicShader.hlsl";
	const std::wstring VSMain = L"VSMainQuad";
	const std::wstring PSMain = L"PSMainQuad";
	m_vsID = m_pRender->LoadShader(ShaderType::VERTEX_SHADER, shaderfile, VSMain, std::wstring(L"vs_5_0"));
	m_psID = m_pRender->LoadShader(ShaderType::PIXEL_SHADER, shaderfile, PSMain, std::wstring(L"ps_5_0"));
}

void HelloWorld::OnResize()
{
	Application::OnResize();
	SetupPipeline();
}

void HelloWorld::BuildGeometry()
{
	// create the vertex buffer resource (this is usually done by GeometryDX11)
	{
		/////////////
		///build quad
		/////////////
		Vertex quadVertices[] =
		{
			{ Vector3f(-1.0f, +1.0f, 0.0f), Colors::White },
			{ Vector3f(+1.0f, +1.0f, 0.0f), Colors::Red },
			{ Vector3f(-1.0f, -1.0f, 0.0f), Colors::Green },
			{ Vector3f(+1.0f, -1.0f, 0.0f), Colors::Blue }
		};

		Subresource data;
		data.data = quadVertices;
		data.rowPitch = 0;
		data.slicePitch = 0;

		BufferConfigDX11 vbConfig;
		vbConfig.SetDefaultVertexBuffer(4 * sizeof(Vertex), false);
		m_pVertexBuffer = m_pRender->CreateVertexBuffer(&vbConfig, &data);
		if (m_pVertexBuffer->m_iResource == -1)
		{
			Log::Get().Write(L"Failed to create vertex buffer");
			assert(false);
		}


		// create the vertex buffer layout descriptor (this is usually done by GeometryDX11)
		D3D11_INPUT_ELEMENT_DESC desc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
		layout.push_back(desc[0]);
		layout.push_back(desc[1]);
		m_VertexLayout = m_pRender->CreateInputLayout(layout, m_vsID);
		if (m_VertexLayout == -1)
		{
			Log::Get().Write(L"Failed to create vertex layout");
			assert(false);
		}
	}

	SetupPipeline();
}

void HelloWorld::SetupPipeline()
{
	InputAssemblerStateDX11 iaState;
	iaState.PrimitiveTopology.SetState(PT_TRIANGLESTRIP);
	iaState.InputLayout.SetState(m_VertexLayout);
	iaState.VertexBuffers.SetState(0, m_pVertexBuffer->m_iResource);
	iaState.VertexBufferStrides.SetState(0, sizeof(Vertex));
	iaState.VertexBufferOffsets.SetState(0, 0);
	iaState.SetFeautureLevel(m_pRender->GetAvailableFeatureLevel(D3D_DRIVER_TYPE_UNKNOWN));
	m_pRender->pImmPipeline->InputAssemblerStage.DesiredState = iaState;

	ShaderStageStateDX11 vsState;
	vsState.ShaderProgram.SetState(m_vsID);
	m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	ShaderStageStateDX11 psState;
	psState.ShaderProgram.SetState(m_psID);
	m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
}