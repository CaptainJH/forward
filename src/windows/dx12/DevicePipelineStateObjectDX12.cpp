//***************************************************************************************
// DevicePipelineStateObjectDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DevicePipelineStateObjectDX12.h"
#include "dxCommon/ShaderDX.h"
#include "FrameGraph/PipelineStateObjects.h"
#include "dx12/ShaderSystem/ShaderDX12.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTextureCubeDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

DevicePipelineStateObjectDX12::DevicePipelineStateObjectDX12(RendererDX12* render, const PipelineStateObject& pso)
	: DeviceObject(nullptr)
	, m_numElements(0)
	, m_pso(pso)
{
	ZeroMemory(&m_elements[0], VA_MAX_ATTRIBUTES * sizeof(m_elements[0]));
	auto device = render->GetDevice();
	auto commandList = render->CommandList();

	FrameGraphVertexBuffer* vbuffer = pso.m_IAState.m_vertexBuffers[0].get();
	FrameGraphVertexShader* vsshader = pso.m_VSState.m_shader.get();
	forward::shared_ptr<ShaderDX12> deviceVS = nullptr;
	if (pso.m_VSState.m_shader)
	{
		deviceVS = device_cast<ShaderDX12*>(pso.m_VSState.m_shader);
	}
	forward::shared_ptr<ShaderDX12> deviceGS = nullptr;
	if (pso.m_GSState.m_shader)
	{
		deviceGS = device_cast<ShaderDX12*>(pso.m_GSState.m_shader);
	}
	forward::shared_ptr<ShaderDX12> devicePS = nullptr;
	if (pso.m_PSState.m_shader)
	{
		devicePS = device_cast<ShaderDX12*>(pso.m_PSState.m_shader);
	}
	FrameGraphGeometryShader* gsshader = pso.m_GSState.m_shader.get();
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
			element.SemanticName = VertexFormat::GetSemanticName(semantic);
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

	if (gsshader && !gsshader->DeviceObject())
	{
		deviceGS = forward::make_shared<ShaderDX12>(gsshader);
		gsshader->SetDeviceObject(deviceGS);
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
		if (gsshader && gsshader->DeviceObject())
		{
			psoDesc.GS =
			{
				reinterpret_cast<BYTE*>(deviceGS->GetCompiledCode()->GetBufferPointer()),
				deviceGS->GetCompiledCode()->GetBufferSize()
			};
		}
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(devicePS->GetCompiledCode()->GetBufferPointer()),
			devicePS->GetCompiledCode()->GetBufferSize()
		};
		ConfigRasterizerState(psoDesc.RasterizerState);
		ConfigBlendState(psoDesc.BlendState);
		ConfigDepthStencilState(psoDesc.DepthStencilState);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = Convert2DX12TopologyType(pso.m_IAState.m_topologyType);
		psoDesc.NumRenderTargets = 0;
		for (auto i = 0U; i < pso.m_OMState.m_renderTargetResources.size(); ++i)
		{
			auto rt = pso.m_OMState.m_renderTargetResources[i];
			if (rt)
			{
				++psoDesc.NumRenderTargets;
				psoDesc.RTVFormats[i] = static_cast<DXGI_FORMAT>(rt->GetFormat());
			}
		}
		if (!psoDesc.NumRenderTargets)
		{
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleDesc.Quality = 0;
		}
		else
		{
			psoDesc.SampleDesc.Count = pso.m_OMState.m_renderTargetResources[0]->GetSampCount();
			psoDesc.SampleDesc.Quality = 0;
		}
		if (pso.m_OMState.m_depthStencilResource)
		{
			psoDesc.DSVFormat = static_cast<DXGI_FORMAT>(pso.m_OMState.m_depthStencilResource->GetFormat());
		}
		HR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_devicePSO)));
	}

	// setup vertex buffers
	render->ResetCommandList();
	for (auto i = 0U; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		auto vb = pso.m_IAState.m_vertexBuffers[i];
		if (vb)
		{
			if (!vb->DeviceObject())
			{
				auto deviceVB = forward::make_shared<DeviceBufferDX12>(device, commandList, vb.get());
				vb->SetDeviceObject(deviceVB);
			}

			//auto deviceVB = device_cast<DeviceBufferDX12*>(vb);
			//deviceVB->SyncCPUToGPU(m_pContext.Get());
			//deviceVB->Bind(m_pContext.Get());
		}
	}
	// setup index buffer
	auto ib = pso.m_IAState.m_indexBuffer;
	if (ib && !ib->DeviceObject())
	{
		auto deviceIB = forward::make_shared<DeviceBufferDX12>(device, commandList, ib.get());
		ib->SetDeviceObject(deviceIB);
	}
	// setup shader resources
	if (pso.m_PSState.m_shader)
	{
		for (auto i = 0U; i < pso.m_PSState.m_shaderResources.size(); ++i)
		{
			auto res = pso.m_PSState.m_shaderResources[i];
			if (res)
			{
				if (dynamic_cast<FrameGraphTexture2D*>(res.get()))
				{
					auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(device, dynamic_cast<FrameGraphTexture2D*>(res.get()));
					res->SetDeviceObject(deviceTex);
				}
				else if (dynamic_cast<FrameGraphTextureCube*>(res.get()))
				{
					auto deviceTex = forward::make_shared<DeviceTextureCubeDX12>(device, dynamic_cast<FrameGraphTextureCube*>(res.get()));
					res->SetDeviceObject(deviceTex);
				}
			}
		}
	}
	// setup render targets
	std::for_each(pso.m_OMState.m_renderTargetResources.begin(), pso.m_OMState.m_renderTargetResources.end(), [&](forward::shared_ptr< FrameGraphTexture2D> ptr) {
		if (ptr && !ptr->DeviceObject())
		{
			auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(device, ptr.get());
			ptr->SetDeviceObject(deviceTex);
		}
	});
	// setup depth stencil buffer
	if (pso.m_OMState.m_depthStencilResource && !pso.m_OMState.m_depthStencilResource->DeviceObject())
	{
		auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(device, pso.m_OMState.m_depthStencilResource.get());
		pso.m_OMState.m_depthStencilResource->SetDeviceObject(deviceTex);
	}

	// Execute the initialization commands
	HR(commandList->Close());
	ID3D12CommandList* cmdLists[] = { commandList };
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
	commandList->SetPipelineState(m_devicePSO.Get());
	for (auto i = 0U; i < m_pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		if (m_pso.m_IAState.m_vertexBuffers[i])
		{
			auto vbv = device_cast<DeviceBufferDX12*>(m_pso.m_IAState.m_vertexBuffers[i])->VertexBufferView();
			commandList->IASetVertexBuffers(i, 1, &vbv);
		}
	}
	if (m_pso.m_IAState.m_indexBuffer)
	{
		auto ibv = device_cast<DeviceBufferDX12*>(m_pso.m_IAState.m_indexBuffer)->IndexBufferView();
		commandList->IASetIndexBuffer(&ibv);
	}
	commandList->IASetPrimitiveTopology(Convert2D3DTopology(m_pso.m_IAState.m_topologyType));
}

void DevicePipelineStateObjectDX12::BuildRootSignature(ID3D12Device* device)
{
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;
	const auto MaxDescriptor = 256;
	std::array<CD3DX12_DESCRIPTOR_RANGE, MaxDescriptor> descriptorRanges;
	memset(&descriptorRanges, 0, descriptorRanges.size());
	auto numDescriptorUsed = 0;

	if (m_pso.m_VSState.m_shader)
	{
		for (auto i = 0U; i < m_pso.m_VSState.m_constantBuffers.size(); ++i)
		{
			if (m_pso.m_VSState.m_constantBuffers[i])
			{
				auto& cbvTable = descriptorRanges[numDescriptorUsed++];
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
				auto& cbvTable = descriptorRanges[numDescriptorUsed++];
				cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i + static_cast<u32>(slotRootParameters.size()));
				CD3DX12_ROOT_PARAMETER param;
				param.InitAsDescriptorTable(1, &cbvTable);
				slotRootParameters.push_back(param);
			}
		}

		for (auto i = 0U; i < m_pso.m_PSState.m_shaderResources.size(); ++i)
		{
			if (m_pso.m_PSState.m_shaderResources[i])
			{
				auto& texTable = descriptorRanges[numDescriptorUsed++];
				texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i);
				CD3DX12_ROOT_PARAMETER param;
				param.InitAsDescriptorTable(1, &texTable);
				slotRootParameters.push_back(param);
			}
		}
	}
	assert(numDescriptorUsed <= MaxDescriptor);

	// A root signature is an array of root parameters.
	const u32 numParameters = static_cast<u32>(slotRootParameters.size());
	auto samplers = ConfigStaticSamplerStates();
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(numParameters, slotRootParameters.empty() ? nullptr : &*slotRootParameters.begin(), 
		static_cast<u32>(samplers.size()), samplers.empty() ? nullptr : &*samplers.begin(), 
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

	HR(device->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE DevicePipelineStateObjectDX12::Convert2DX12TopologyType(PrimitiveTopologyType topo)
{
	if (topo == PT_UNDEFINED)
	{
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
	else if (topo == PT_POINTLIST)
	{
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	}
	else if (topo == PT_LINELIST || topo == PT_LINESTRIP 
		|| topo == PT_LINELIST_ADJ || topo == PT_LINESTRIP_ADJ)
	{
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	}
	else if (topo == PT_TRIANGLELIST || topo == PT_TRIANGLESTRIP
		|| topo == PT_TRIANGLELIST_ADJ || topo == PT_TRIANGLESTRIP_ADJ)
	{
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

void DevicePipelineStateObjectDX12::ConfigRasterizerState(D3D12_RASTERIZER_DESC& desc) const
{
	desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	auto& rsState = m_pso.m_RSState.m_rsState;
	desc.FillMode = msFillMode[rsState.fillMode];
	desc.CullMode = msCullMode[rsState.cullMode];
	desc.FrontCounterClockwise = rsState.frontCCW ? TRUE : FALSE;
	desc.DepthBias = rsState.depthBias;
	desc.DepthBiasClamp = rsState.depthBiasClamp;
	desc.SlopeScaledDepthBias = rsState.slopeScaledDepthBias;
	desc.DepthClipEnable = rsState.enableDepthClip ? TRUE : FALSE;
	desc.MultisampleEnable = rsState.enableMultisample ? TRUE : FALSE;
	desc.AntialiasedLineEnable = rsState.enableAntialiasedLine ? TRUE : FALSE;
}

void DevicePipelineStateObjectDX12::ConfigBlendState(D3D12_BLEND_DESC& desc) const
{
	desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	auto& blendState = m_pso.m_OMState.m_blendState;
	desc.AlphaToCoverageEnable = blendState.enableAlphaToCoverage ? TRUE : FALSE;
	desc.IndependentBlendEnable = blendState.enableIndependentBlend ? TRUE : FALSE;
	for (auto i = 0U; i < BlendState::NUM_TARGETS; ++i)
	{
		D3D12_RENDER_TARGET_BLEND_DESC& out = desc.RenderTarget[i];
		const BlendState::Target& in = blendState.target[i];
		out.BlendEnable = in.enable ? TRUE : FALSE;
		out.SrcBlend = msBlendMode[in.srcColor];
		out.DestBlend = msBlendMode[in.dstColor];
		out.BlendOp = msBlendOp[in.opColor];
		out.SrcBlendAlpha = msBlendMode[in.srcAlpha];
		out.DestBlendAlpha = msBlendMode[in.dstAlpha];
		out.BlendOpAlpha = msBlendOp[in.opAlpha];
		out.RenderTargetWriteMask = in.mask;
	}
}

void DevicePipelineStateObjectDX12::ConfigDepthStencilState(D3D12_DEPTH_STENCIL_DESC& desc) const
{
	desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	auto& dsState = m_pso.m_OMState.m_dsState;
	desc.DepthEnable = dsState.depthEnable ? TRUE : FALSE;
	desc.DepthWriteMask = msWriteMask[dsState.writeMask];
	desc.DepthFunc = msComparison[dsState.comparison];
	desc.StencilEnable = dsState.stencilEnable ? TRUE : FALSE;
	desc.StencilReadMask = dsState.stencilReadMask;
	desc.StencilWriteMask = dsState.stencilWriteMask;
	DepthStencilState::Face front = dsState.frontFace;
	desc.FrontFace.StencilFailOp = msStencilOp[front.fail];
	desc.FrontFace.StencilDepthFailOp = msStencilOp[front.depthFail];
	desc.FrontFace.StencilPassOp = msStencilOp[front.pass];
	desc.FrontFace.StencilFunc = msComparison[front.comparison];
	DepthStencilState::Face back = dsState.backFace;
	desc.BackFace.StencilFailOp = msStencilOp[back.fail];
	desc.BackFace.StencilDepthFailOp = msStencilOp[back.depthFail];
	desc.BackFace.StencilPassOp = msStencilOp[back.pass];
	desc.BackFace.StencilFunc = msComparison[back.comparison];
}

std::vector<CD3DX12_STATIC_SAMPLER_DESC> DevicePipelineStateObjectDX12::ConfigStaticSamplerStates() const
{
	std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers;

	for (auto i = 0U; i < m_pso.m_PSState.m_samplers.size(); ++i)
	{
		auto samp = m_pso.m_PSState.m_samplers[i];
		if (samp)
		{
			CD3DX12_STATIC_SAMPLER_DESC desc(
				i, msFilter[samp->filter],
				msAddressMode[samp->mode[0]],
				msAddressMode[samp->mode[1]],
				msAddressMode[samp->mode[2]],
				samp->mipLODBias,
				samp->maxAnisotropy,
				msComparison[samp->comparison],
				D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
				samp->minLOD,
				samp->maxLOD
			);

			samplers.push_back(desc);
		}
	}

	return samplers;
}

D3D12_FILL_MODE const DevicePipelineStateObjectDX12::msFillMode[] =
{
	D3D12_FILL_MODE_SOLID,
	D3D12_FILL_MODE_WIREFRAME
};

D3D12_CULL_MODE const DevicePipelineStateObjectDX12::msCullMode[] =
{
	D3D12_CULL_MODE_NONE,
	D3D12_CULL_MODE_FRONT,
	D3D12_CULL_MODE_BACK
};

D3D12_BLEND const DevicePipelineStateObjectDX12::msBlendMode[] =
{
	D3D12_BLEND_ZERO,
	D3D12_BLEND_ONE,
	D3D12_BLEND_SRC_COLOR,
	D3D12_BLEND_INV_SRC_COLOR,
	D3D12_BLEND_SRC_ALPHA,
	D3D12_BLEND_INV_SRC_ALPHA,
	D3D12_BLEND_DEST_ALPHA,
	D3D12_BLEND_INV_DEST_ALPHA,
	D3D12_BLEND_DEST_COLOR,
	D3D12_BLEND_INV_DEST_COLOR,
	D3D12_BLEND_SRC_ALPHA_SAT,
	D3D12_BLEND_BLEND_FACTOR,
	D3D12_BLEND_INV_BLEND_FACTOR,
	D3D12_BLEND_SRC1_COLOR,
	D3D12_BLEND_INV_SRC1_COLOR,
	D3D12_BLEND_SRC1_ALPHA,
	D3D12_BLEND_INV_SRC1_ALPHA
};

D3D12_BLEND_OP const DevicePipelineStateObjectDX12::msBlendOp[] =
{
	D3D12_BLEND_OP_ADD,
	D3D12_BLEND_OP_SUBTRACT,
	D3D12_BLEND_OP_REV_SUBTRACT,
	D3D12_BLEND_OP_MIN,
	D3D12_BLEND_OP_MAX,
};

D3D12_DEPTH_WRITE_MASK const DevicePipelineStateObjectDX12::msWriteMask[] =
{
	D3D12_DEPTH_WRITE_MASK_ZERO,
	D3D12_DEPTH_WRITE_MASK_ALL
};

D3D12_COMPARISON_FUNC const DevicePipelineStateObjectDX12::msComparison[] =
{
	D3D12_COMPARISON_FUNC_NEVER,
	D3D12_COMPARISON_FUNC_LESS,
	D3D12_COMPARISON_FUNC_EQUAL,
	D3D12_COMPARISON_FUNC_LESS_EQUAL,
	D3D12_COMPARISON_FUNC_GREATER,
	D3D12_COMPARISON_FUNC_NOT_EQUAL,
	D3D12_COMPARISON_FUNC_GREATER_EQUAL,
	D3D12_COMPARISON_FUNC_ALWAYS
};

D3D12_STENCIL_OP const DevicePipelineStateObjectDX12::msStencilOp[] =
{
	D3D12_STENCIL_OP_KEEP,
	D3D12_STENCIL_OP_ZERO,
	D3D12_STENCIL_OP_REPLACE,
	D3D12_STENCIL_OP_INCR_SAT,
	D3D12_STENCIL_OP_DECR_SAT,
	D3D12_STENCIL_OP_INVERT,
	D3D12_STENCIL_OP_INCR,
	D3D12_STENCIL_OP_DECR
};

D3D12_FILTER const DevicePipelineStateObjectDX12::msFilter[] =
{
	D3D12_FILTER_MIN_MAG_MIP_POINT,
	D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
	D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
	D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
	D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
	D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	D3D12_FILTER_ANISOTROPIC,
	D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
	D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
	D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
	D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
	D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
	D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
	D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
	D3D12_FILTER_COMPARISON_ANISOTROPIC
};

D3D12_TEXTURE_ADDRESS_MODE const DevicePipelineStateObjectDX12::msAddressMode[] =
{
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_BORDER,
	D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE
};