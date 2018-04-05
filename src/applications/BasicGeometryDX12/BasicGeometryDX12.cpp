#include "dx12/ApplicationDX12.h"
#include "Vector3f.h"
#include "dx12/UploadBuffer.h"
#include "dxCommon/ShaderFactoryDX.h"

//#include "ResourceSystem\Buffer\BufferConfigDX11.h"

#pragma warning(disable : 4238)

using namespace forward;

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

class BasicGeometryDX12 : public ApplicationDX12
{
public:
	BasicGeometryDX12(HINSTANCE hInstance, i32 width, i32 height)
		: ApplicationDX12(hInstance, width, height)
	{
		mMainWndCaption = L"BasicGeometryDX12!";
	}

	~BasicGeometryDX12()
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

	Matrix4f m_worldMat;
	Matrix4f m_viewMat;
	Matrix4f m_projMat;
};

i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, i32 /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BasicGeometryDX12 theApp(hInstance, 800, 600);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

void BasicGeometryDX12::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 1000;
	m_worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
}

void BasicGeometryDX12::DrawScene()
{
	auto mat = m_worldMat * m_viewMat * m_projMat;
	ObjectConstants constants;
	constants.WorldViewProj = mat;
	m_objectCB->CopyData(0, constants);

	m_pRender->BeginPresent(m_pso.Get());

	auto commandList = m_pRender->CommandList();
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	auto vbv = m_geometry->VertexBufferView();
	auto ibv = m_geometry->IndexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetIndexBuffer(&ibv);
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	m_pRender->EndPresent();
}

bool BasicGeometryDX12::Init()
{
	Log::Get().Open();
	if (!ApplicationDX12::Init())
		return false;

	// init world matrix
	m_worldMat = Matrix4f::Identity();
	// build view matrix
	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
	//build projection matrix
	m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);


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

void BasicGeometryDX12::BuildShadersAndInputLayout()
{
	const std::wstring shaderfile = L"BasicShader.hlsl";
	const std::wstring VSMain = L"VSMain";
	const std::wstring PSMain = L"PSMain";

	m_vsByteCode = ShaderFactoryDX::GenerateShader(ShaderType::VERTEX_SHADER, shaderfile, VSMain, std::wstring(L"vs_5_0"));
	m_psByteCode = ShaderFactoryDX::GenerateShader(ShaderType::PIXEL_SHADER, shaderfile, PSMain, std::wstring(L"ps_5_0"));

	m_inputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void BasicGeometryDX12::OnResize()
{
	ApplicationDX12::OnResize();
	SetupPipeline();
}

void BasicGeometryDX12::BuildGeometry()
{
	// create the vertex buffer resource 
	{
		/////////////
		///build box
		/////////////
		Vertex boxVertices[] =
		{
			{ Vector3f(-1.0f, -1.0f, -1.0f), Colors::White },
			{ Vector3f(-1.0f, +1.0f, -1.0f), Colors::Black },
			{ Vector3f(+1.0f, +1.0f, -1.0f), Colors::Red },
			{ Vector3f(+1.0f, -1.0f, -1.0f), Colors::Green },
			{ Vector3f(-1.0f, -1.0f, +1.0f), Colors::Blue },
			{ Vector3f(-1.0f, +1.0f, +1.0f), Colors::Yellow },
			{ Vector3f(+1.0f, +1.0f, +1.0f), Colors::Cyan },
			{ Vector3f(+1.0f, -1.0f, +1.0f), Colors::Magenta}
		};

		u16 indices[] = 
		{
			0, 1, 2,
			0, 2, 3,
				   
			4, 6, 5,
			4, 7, 6,
				   
			4, 5, 1,
			4, 1, 0,
				   
			3, 2, 6,
			3, 6, 7,
				   
			1, 5, 6,
			1, 6, 2,
				   
			4, 0, 3,
			4, 3, 7,
		};

		const u32 vbByteSize = _countof(boxVertices) * sizeof(Vertex);
		const u32 ibByteSize = _countof(indices) * sizeof(u16);
		m_geometry = std::make_unique<MeshGeometry>();
		m_geometry->Name = "box";

		HR(D3DCreateBlob(vbByteSize, &m_geometry->VertexBufferCPU));
		CopyMemory(m_geometry->VertexBufferCPU->GetBufferPointer(), boxVertices, vbByteSize);
		HR(D3DCreateBlob(ibByteSize, &m_geometry->IndexBufferCPU));
		CopyMemory(m_geometry->IndexBufferCPU->GetBufferPointer(), indices, ibByteSize);

		m_geometry->VertexBufferGPU = CreateDefaultBuffer(m_pRender->GetDevice(),
			m_pRender->CommandList(), boxVertices, vbByteSize, m_geometry->VertexBufferUploader);
		m_geometry->IndexBufferGPU = CreateDefaultBuffer(m_pRender->GetDevice(),
			m_pRender->CommandList(), indices, ibByteSize, m_geometry->IndexBufferUploader);

		m_geometry->VertexByteStride = sizeof(Vertex);
		m_geometry->VertexBufferByteSize = vbByteSize;
		m_geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
		m_geometry->IndexBufferByteSize = ibByteSize;
		
		SubmeshGeometry submesh;
		submesh.IndexCount = 0;
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		m_geometry->DrawArgs["screen"] = submesh;
	}
}

void BasicGeometryDX12::SetupPipeline()
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

void BasicGeometryDX12::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	HR(m_pRender->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void BasicGeometryDX12::BuildConstantBuffers()
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

void BasicGeometryDX12::BuildRootSignature()
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

void BasicGeometryDX12::BuildPSO()
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