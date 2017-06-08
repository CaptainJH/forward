#include "dx12/ApplicationDX12.h"
#include "Vector3f.h"
#include "dx12/UploadBuffer.h"
#include "dx12/d3dx12.h"
#include "dxCommon/ShaderFactoryDX.h"

//#include "ResourceSystem\Buffer\BufferConfigDX11.h"

#pragma warning(disable : 4238)

using namespace forward;

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	ResourceComPtr defaultBuffer;

	// Create the actual default buffer resource.
	HR(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	HR(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;


	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
};

struct ObjectConstants
{
	Matrix4f WorldViewProj = Matrix4f::Identity();
};

class HelloDX12 : public ApplicationDX12
{
public:
	HelloDX12(HINSTANCE hInstance, i32 width, i32 height)
		: ApplicationDX12(hInstance, width, height)
	{
		mMainWndCaption = L"Hello DirectX12!";
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
	void SetupPipeline();

	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildPSO();

	RootSignatureComPtr m_rootSignature = nullptr;
	DescriptorHeapComPtr m_cbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants> > m_objectCB = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	PipelineStateComPtr m_pso = nullptr;

	std::unique_ptr<MeshGeometry> m_geometry = nullptr;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HelloDX12 theApp(hInstance, 800, 600);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void HelloDX12::UpdateScene(f32 /*dt*/)
{
}

void HelloDX12::DrawScene()
{
	m_pRender->BeginPresent(m_pso.Get());

	auto commandList = m_pRender->CommandList();
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	auto vbv = m_geometry->VertexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->DrawInstanced(4, 1, 0, 0);

	m_pRender->EndPresent();
}

bool HelloDX12::Init()
{
	Log::Get().Open();
	if (!ApplicationDX12::Init())
		return false;

	m_pRender->ResetCommandList();

	BuildDescriptorHeaps();
	BuildConstantBuffers();
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

void HelloDX12::BuildShadersAndInputLayout()
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

void HelloDX12::OnResize()
{
	ApplicationDX12::OnResize();
	SetupPipeline();
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

void HelloDX12::SetupPipeline()
{
	//InputAssemblerStateDX11 iaState;
	//iaState.PrimitiveTopology.SetState(PT_TRIANGLESTRIP);
	//iaState.InputLayout.SetState(m_VertexLayout);
	//iaState.VertexBuffers.SetState(0, m_pVertexBuffer->m_iResource);
	//iaState.VertexBufferStrides.SetState(0, sizeof(Vertex));
	//iaState.VertexBufferOffsets.SetState(0, 0);
	//iaState.SetFeautureLevel(m_pRender->GetAvailableFeatureLevel(D3D_DRIVER_TYPE_UNKNOWN));
	//m_pRender->pImmPipeline->InputAssemblerStage.DesiredState = iaState;

	//ShaderStageStateDX11 vsState;
	//vsState.ShaderProgram.SetState(m_vsID);
	//m_pRender->pImmPipeline->VertexShaderStage.DesiredState = vsState;

	//ShaderStageStateDX11 psState;
	//psState.ShaderProgram.SetState(m_psID);
	//m_pRender->pImmPipeline->PixelShaderStage.DesiredState = psState;
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

void HelloDX12::BuildConstantBuffers()
{
	m_objectCB = std::make_unique<UploadBuffer<ObjectConstants> >(m_pRender->GetDevice(), 1, true);

	u32 objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_objectCB->Resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	i32 boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objCBByteSize;

	m_pRender->GetDevice()->CreateConstantBufferView(&cbvDesc,
		m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
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