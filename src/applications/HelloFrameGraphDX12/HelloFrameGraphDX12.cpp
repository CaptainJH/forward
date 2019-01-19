#include "ApplicationWin.h"
#include "Vector3f.h"
#include "dx12/UploadBuffer.h"
#include "dxCommon/ShaderFactoryDX.h"
#include "dx12/RendererDX12.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
};

class HelloFrameGraphDX12 : public Application
{
public:
	HelloFrameGraphDX12(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
	{
		mMainWndCaption = L"Hello DirectX12!";
		RenderType = RendererType::Renderer_Forward_DX12;
	}

	~HelloFrameGraphDX12()
	{
		Log::Get().Close();
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	virtual void UpdateScene(f32 dt);
	virtual void DrawScene();

private:
	void BuildShadersAndInputLayout();
	void BuildGeometry();

	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildPSO();

	RootSignatureComPtr m_rootSignature = nullptr;
	DescriptorHeapComPtr m_cbvHeap = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	PipelineStateComPtr m_pso = nullptr;

	std::unique_ptr<MeshGeometry> m_geometry = nullptr;

	RendererDX12* m_pRender = nullptr;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HelloFrameGraphDX12 theApp(hInstance, 800, 600);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void HelloFrameGraphDX12::UpdateScene(f32 /*dt*/)
{
}

void HelloFrameGraphDX12::DrawScene()
{
	m_pRender->BeginPresent(m_pso.Get());

	auto commandList = m_pRender->CommandList();
	//ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	//commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	auto vbv = m_geometry->VertexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->DrawInstanced(4, 1, 0, 0);

	m_pRender->EndPresent();
}

bool HelloFrameGraphDX12::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_pRender = dynamic_cast<RendererDX12*>(m_pRender2);

	//BuildDescriptorHeaps();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildPSO();

	// Execute the initialization commands
	HR(m_pRender->CommandList()->Close());
	ID3D12CommandList* cmdLists[] = { m_pRender->CommandList() };
	m_pRender->CommandQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// Wait until initialization is complete.
	m_pRender->FlushCommandQueue();


	return true;
}

void HelloFrameGraphDX12::BuildShadersAndInputLayout()
{
	const std::wstring shaderfile = L"BasicShader.hlsl";
	const std::wstring VSMain = L"VSMainQuad";
	const std::wstring PSMain = L"PSMainQuad";

	m_vsByteCode = ShaderFactoryDX::GenerateShader(ShaderType::VERTEX_SHADER, shaderfile, VSMain, std::wstring(L"vs_5_0"));
	m_psByteCode = ShaderFactoryDX::GenerateShader(ShaderType::PIXEL_SHADER, shaderfile, PSMain, std::wstring(L"ps_5_0"));

	m_inputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void HelloFrameGraphDX12::OnResize()
{
	Application::OnResize();
}

void HelloFrameGraphDX12::BuildGeometry()
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

		const u32 vbByteSize = 4 * sizeof(Vertex);
		m_geometry = std::make_unique<MeshGeometry>();
		m_geometry->Name = "screen";

		//HR(D3DCreateBlob(vbByteSize, &m_geometry->VertexBufferCPU));
		//CopyMemory(m_geometry->VertexBufferCPU->GetBufferPointer(), quadVertices, vbByteSize);
		m_geometry->VertexBufferGPU = CreateDefaultBuffer(m_pRender->GetDevice(),
			m_pRender->CommandList(), quadVertices, vbByteSize, m_geometry->VertexBufferUploader);

		m_geometry->VertexByteStride = sizeof(Vertex);
		m_geometry->VertexBufferByteSize = vbByteSize;
		
		SubmeshGeometry submesh;
		submesh.IndexCount = 0;
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		m_geometry->DrawArgs["screen"] = submesh;
	}
}

void HelloFrameGraphDX12::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	HR(m_pRender->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void HelloFrameGraphDX12::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HR(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	HR(m_pRender->GetDevice()->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

void HelloFrameGraphDX12::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (u32)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = 
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS = 
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	HR(m_pRender->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}