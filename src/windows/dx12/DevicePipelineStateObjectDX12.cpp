//***************************************************************************************
// DevicePipelineStateObjectDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DevicePipelineStateObjectDX12.h"
#include "dxCommon/ShaderDX.h"
#include "FrameGraph/PipelineStateObjects.h"
#include "dx12/ShaderSystem/ShaderDX12.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

DevicePipelineStateObjectDX12::DevicePipelineStateObjectDX12(RendererDX12* render, const PipelineStateObject& pso)
	: DeviceObject(nullptr)
	, m_numElements(0)
	, m_pso(pso)
{
	ZeroMemory(&m_elements[0], VA_MAX_ATTRIBUTES * sizeof(m_elements[0]));
	auto device = render->GetDevice();
	auto commandLIst = render->CommandList();

	FrameGraphVertexBuffer* vbuffer = pso.m_IAState.m_vertexBuffers[0].get();
	FrameGraphVertexShader* vsshader = pso.m_VSState.m_shader.get();
	forward::shared_ptr<ShaderDX12> deviceVS;
	forward::shared_ptr<ShaderDX12> devicePS;
	FrameGraphPixelShader* psshader = pso.m_PSState.m_shader.get();

	if (vbuffer && vsshader)
	{
		const auto& vertexFormat = vbuffer->GetVertexFormat();
		m_numElements = vertexFormat.GetNumAttributes();
		for (auto i = 0U; i < m_numElements; ++i)
		{
			VASemantic semantic;
			DataFormatType type;
			u32 unit, offset;
			vertexFormat.GetAttribute(i, semantic, type, unit, offset);

			D3D12_INPUT_ELEMENT_DESC& element = m_elements[i];
			element.SemanticName = msSemantic[semantic];
			element.SemanticIndex = unit;
			element.Format = static_cast<DXGI_FORMAT>(type);
			element.InputSlot = 0;  // TODO: Streams not yet supported.
			element.AlignedByteOffset = offset;
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
		}
	}

	// prepare shaders
	if (!vsshader->DeviceObject())
	{
		deviceVS = forward::make_shared<ShaderDX12>(vsshader);
		vsshader->SetDeviceObject(deviceVS);
	}

	if (!psshader->DeviceObject())
	{
		devicePS = forward::make_shared<ShaderDX12>(psshader);
		psshader->SetDeviceObject(devicePS);
	}

	// setup root signature
	if (!m_rootSignature.Get())
	{
		BuildRootSignature(device);
	}

	// setup pso
	if (!m_devicePSO.Get())
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { m_elements, m_numElements };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(deviceVS->GetCompiledCode()->GetBufferPointer()),
			deviceVS->GetCompiledCode()->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(devicePS->GetCompiledCode()->GetBufferPointer()),
			devicePS->GetCompiledCode()->GetBufferSize()
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
		HR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_devicePSO)));
	}

	// setup vertex buffer
	render->ResetCommandList();
	for (auto i = 0U; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		auto vb = pso.m_IAState.m_vertexBuffers[i];
		if (vb)
		{
			if (!vb->DeviceObject())
			{
				auto deviceVB = forward::make_shared<DeviceBufferDX12>(device, commandLIst, vb.get());
				vb->SetDeviceObject(deviceVB);
			}

			//auto deviceVB = device_cast<DeviceBufferDX12*>(vb);
			//deviceVB->SyncCPUToGPU(m_pContext.Get());
			//deviceVB->Bind(m_pContext.Get());
		}
	}
	// Execute the initialization commands
	HR(commandLIst->Close());
	ID3D12CommandList* cmdLists[] = { commandLIst };
	render->CommandQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// Wait until initialization is complete.
	render->FlushCommandQueue();
}

DevicePipelineStateObjectDX12::~DevicePipelineStateObjectDX12()
{

}

ID3D12PipelineState* DevicePipelineStateObjectDX12::GetDevicePSO()
{
	assert(m_devicePSO.Get());
	return m_devicePSO.Get();
}

void DevicePipelineStateObjectDX12::Bind(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	auto vbv = device_cast<DeviceBufferDX12*>(m_pso.m_IAState.m_vertexBuffers[0])->VertexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void DevicePipelineStateObjectDX12::BuildRootSignature(ID3D12Device* device)
{
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;

	if (m_pso.m_VSState.m_shader)
	{
		for (auto i = 0U; i < m_pso.m_VSState.m_constantBuffers.size(); ++i)
		{
			if (m_pso.m_VSState.m_constantBuffers[i])
			{
				CD3DX12_DESCRIPTOR_RANGE cbvTable;
				cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
				CD3DX12_ROOT_PARAMETER param;
				param.InitAsDescriptorTable(1, &cbvTable);
				slotRootParameters.push_back(param);
			}
		}
	}

	if (m_pso.m_PSState.m_shader)
	{
		for (auto i = 0U; i < m_pso.m_PSState.m_constantBuffers.size(); ++i)
		{
			if (m_pso.m_PSState.m_constantBuffers[i])
			{
				CD3DX12_DESCRIPTOR_RANGE cbvTable;
				cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
				CD3DX12_ROOT_PARAMETER param;
				param.InitAsDescriptorTable(1, &cbvTable);
				slotRootParameters.push_back(param);
			}
		}
	}

	// A root signature is an array of root parameters.
	const u32 numParameters = static_cast<u32>(slotRootParameters.size());
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(numParameters, numParameters > 0 ? &*slotRootParameters.begin() : nullptr, 
		0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HR(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	HR(device->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

i8 const* DevicePipelineStateObjectDX12::msSemantic[VA_NUM_SEMANTICS] =
{
	"",
	"POSITION",
	"BLENDWEIGHT",
	"BLENDINDICES",
	"NORMAL",
	"PSIZE",
	"TEXCOORD",
	"TANGENT",
	"BINORMAL",
	"TESSFACTOR",
	"POSITIONT",
	"COLOR",
	"FOG",
	"DEPTH",
	"SAMPLE"
};