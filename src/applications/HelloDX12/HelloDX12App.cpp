#include "utilities/Application.h"
#include "math/Vector3f.h"
#include "windows/dx12/DeviceDX12.h"
#include "windows/dx12/CommandQueueDX12.h"
#include "windows/dxCommon/ShaderFactoryDX.h"
#include "utilities/FileLoader.h"

#include "utilities/ProfilingHelper.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	float4 Color;
};

class HelloDX12 : public Application
{
public:
	HelloDX12(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
	{
		mMainWndCaption = L"Hello DirectX12!";
		DeviceType = DeviceType::Device_Forward_DX12;
	}

	~HelloDX12()
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
	Vector<u8> m_vsByteCode6;
	Vector<u8> m_psByteCode6;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	PipelineStateComPtr m_pso = nullptr;

	std::unique_ptr<MeshGeometry> m_geometry = nullptr;
	DeviceDX12* m_pRender = nullptr;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HelloDX12 theApp(hInstance, 1920, 1080);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void HelloDX12::UpdateScene(f32 /*dt*/)
{
}

void HelloDX12::DrawScene()
{
	ProfilingHelper::BeginPixEvent("DrawScene");
	m_pRender->BeginDraw();

	auto commandList = m_pRender->DeviceCommandList();
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	commandList->SetPipelineState(m_pso.Get());
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	auto vbv = m_geometry->VertexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->DrawInstanced(4, 1, 0, 0);

	m_pRender->EndDraw();
	ProfilingHelper::EndPixEvent();
}

bool HelloDX12::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_pRender = static_cast<DeviceDX12*>(m_pDevice);

	BuildDescriptorHeaps();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildPSO();

	// Execute the initialization commands
	m_pRender->GetDefaultQueue()->ExecuteCommandList([]() {});
	m_pRender->GetDefaultQueue()->Flush();

	return true;
}

void HelloDX12::BuildShadersAndInputLayout()
{
	const std::wstring shaderfile = L"BasicShader.hlsl";
	const std::string VSMain = "VSMainQuad";
	const std::string PSMain = "PSMainQuad";

	std::wstring filepath = FileSystem::getSingleton().GetShaderFolder() + shaderfile;
	FileLoader sourceFile;
	sourceFile.Open(filepath);
	String shaderText = sourceFile.GetDataPtr();

	//m_vsByteCode = ShaderFactoryDX::GenerateShader(shaderfile, shaderText, VSMain, "vs_5_0");
	//m_psByteCode = ShaderFactoryDX::GenerateShader(shaderfile, shaderText, PSMain, "ps_5_0");

	m_vsByteCode6 = ShaderFactoryDX::GenerateShader6(shaderfile, shaderText, VSMain, "vs_6_0", [](auto, auto){});
	m_psByteCode6 = ShaderFactoryDX::GenerateShader6(shaderfile, shaderText, PSMain, "ps_6_0", [](auto, auto) {});

	m_inputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void HelloDX12::OnResize()
{
	Application::OnResize();
}

void HelloDX12::BuildGeometry()
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

		HR(D3DCreateBlob(vbByteSize, &m_geometry->VertexBufferCPU));
		CopyMemory(m_geometry->VertexBufferCPU->GetBufferPointer(), quadVertices, vbByteSize);
		m_geometry->VertexBufferGPU = CreateDefaultBuffer(m_pRender->GetDevice(),
			m_pRender->DeviceCommandList(), quadVertices, vbByteSize, m_geometry->VertexBufferUploader);

		m_geometry->VertexByteStride = sizeof(Vertex);
		m_geometry->VertexBufferByteSize = vbByteSize;
		
		SubmeshGeometry submesh;
		submesh.IndexCount = 0;
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		m_geometry->DrawArgs["screen"] = submesh;
	}
}

void HelloDX12::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	HR(m_pRender->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void HelloDX12::BuildRootSignature()
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

void HelloDX12::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (u32)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		//reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		//m_vsByteCode->GetBufferSize()
		m_vsByteCode6.data(), m_vsByteCode6.size()
	};
	psoDesc.PS =
	{
		//reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		//m_psByteCode->GetBufferSize()
		m_psByteCode6.data(), m_psByteCode6.size()
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