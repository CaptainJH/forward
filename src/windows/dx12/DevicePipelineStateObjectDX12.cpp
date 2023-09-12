//***************************************************************************************
// DevicePipelineStateObjectDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DevicePipelineStateObjectDX12.h"
#include "dxCommon/ShaderDX.h"
#include "FrameGraph/PipelineStateObjects.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTextureCubeDX12.h"
#include "dx12/DeviceDX12.h"
#include "dx12/CommandQueueDX12.h"

using namespace forward;

DevicePipelineStateObjectDX12::DevicePipelineStateObjectDX12(DeviceDX12* d, PipelineStateObject& pso)
	: DeviceObject(nullptr)
	, m_numElements(0)
	, m_pso(pso)
{
	ZeroMemory(&m_elements[0], VA_MAX_ATTRIBUTES * sizeof(m_elements[0]));
	auto device = d->GetDevice();
	auto commandList = d->DeviceCommandList();

	if (!pso.m_CSState.m_shader)
	{
		// setup graphic pipeline
		VertexBuffer* vbuffer = pso.m_IAState.m_vertexBuffers[0].get();
		VertexShader* vsshader = pso.m_VSState.m_shader.get();
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
		GeometryShader* gsshader = pso.m_GSState.m_shader.get();
		PixelShader* psshader = pso.m_PSState.m_shader.get();

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
				deviceVS->GetCompiledCode(),
				deviceVS->GetCompiledCodeSize()
			};
			if (gsshader && gsshader->DeviceObject())
			{
				psoDesc.GS =
				{
					deviceGS->GetCompiledCode(),
					deviceGS->GetCompiledCodeSize()
				};
			}
			psoDesc.PS =
			{
				devicePS->GetCompiledCode(),
				devicePS->GetCompiledCodeSize()
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
		for (auto i = 0U; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
		{
			auto vb = pso.m_IAState.m_vertexBuffers[i];
			if (vb)
			{
				if (!vb->DeviceObject())
				{
					auto deviceVB = forward::make_shared<DeviceBufferDX12>(commandList, vb.get(), *d);
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
			auto deviceIB = forward::make_shared<DeviceBufferDX12>(commandList, ib.get(), *d);
			ib->SetDeviceObject(deviceIB);
		}
		// setup shader resources
		if (pso.m_PSState.m_shader)
		{
			const auto& allTextures = devicePS->GetTextures();
			std::sort(pso.m_PSState.m_shaderResources.begin(), pso.m_PSState.m_shaderResources.begin() + allTextures.size(), 
				[&](auto& lhs, auto& rhs)->bool {
					return std::find_if(allTextures.begin(), allTextures.end(), [&](auto p)->bool {
						return p.GetName() == lhs->Name();
						}) <
					std::find_if(allTextures.begin(), allTextures.end(), [&](auto p)->bool {
						return p.GetName() == rhs->Name();
						});
				});
			for (auto i = 0U; i < pso.m_PSState.m_shaderResources.size(); ++i)
			{
				auto res = pso.m_PSState.m_shaderResources[i];
				if (res)
				{
					if (dynamic_cast<Texture2D*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dynamic_cast<Texture2D*>(res.get()), *d);
						res->SetDeviceObject(deviceTex);
					}
					else if (dynamic_cast<TextureCube*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTextureCubeDX12>(dynamic_cast<TextureCube*>(res.get()), *d);
						res->SetDeviceObject(deviceTex);
					}
				}
			}
		}
		// setup render targets
		std::for_each(pso.m_OMState.m_renderTargetResources.begin(), pso.m_OMState.m_renderTargetResources.end(), [&](forward::shared_ptr< Texture2D> ptr) {
			if (ptr && !ptr->DeviceObject() && ptr->Name() != "DefaultRT")
			{
				auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(ptr.get(), *d);
				ptr->SetDeviceObject(deviceTex);
			}
			});
		// setup depth stencil buffer
		if (pso.m_OMState.m_depthStencilResource && !pso.m_OMState.m_depthStencilResource->DeviceObject())
		{
			auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(pso.m_OMState.m_depthStencilResource.get(), *d);
			pso.m_OMState.m_depthStencilResource->SetDeviceObject(deviceTex);
		}

		// Execute the initialization commands
		d->GetDefaultQueue()->ExecuteCommandList([]() {});
		d->GetDefaultQueue()->Flush();
	}
	else if (pso.m_CSState.m_shader)
	{
		// setup compute pipeline
		ComputeShader* csshader = pso.m_CSState.m_shader.get();
		forward::shared_ptr<ShaderDX12> deviceCS = nullptr;
		if (!csshader->DeviceObject())
		{
			deviceCS = forward::make_shared<ShaderDX12>(csshader);
			csshader->SetDeviceObject(deviceCS);
		}

		// setup root signature
		if (!m_rootSignature.Get())
			BuildRootSignature(device);

		// setup pso
		if (!m_devicePSO.Get())
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.CS =
			{
				deviceCS->GetCompiledCode(),
				deviceCS->GetCompiledCodeSize()
			};
			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			HR(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_devicePSO)));
		}

		// setup shader resources
		if (pso.m_CSState.m_shader)
		{
			for (auto i = 0U; i < pso.m_CSState.m_shaderResources.size(); ++i)
			{
				auto res = pso.m_CSState.m_shaderResources[i];
				if (res)
				{
					if (dynamic_cast<Texture2D*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dynamic_cast<Texture2D*>(res.get()), *d);
						res->SetDeviceObject(deviceTex);
					}
					else if (dynamic_cast<TextureCube*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTextureCubeDX12>(dynamic_cast<TextureCube*>(res.get()), *d);
						res->SetDeviceObject(deviceTex);
					}
				}
			}
			for (auto i = 0U; i < pso.m_CSState.m_uavShaderRes.size(); ++i)
			{
				auto res = pso.m_CSState.m_uavShaderRes[i];
				if (res)
				{
					if (dynamic_cast<Texture2D*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dynamic_cast<Texture2D*>(res.get()), *d);
						res->SetDeviceObject(deviceTex);
					}
				}
			}
		}

	}
}

DevicePipelineStateObjectDX12::~DevicePipelineStateObjectDX12()
{

}

ID3D12PipelineState* DevicePipelineStateObjectDX12::GetDevicePSO()
{
	assert(m_devicePSO.Get());
	return m_devicePSO.Get();
}

void DevicePipelineStateObjectDX12::BuildRootSignature(ID3D12Device* device)
{
	const auto MaxDescriptor = 256;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;
	descriptorRanges.reserve(MaxDescriptor);
	std::array<u32, FORWARD_RENDERER_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> usedRegisterCBV{ 0 };
	std::array<u32, FORWARD_RENDERER_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> usedRegisterSRV{ 0 };
	std::array<u32, 8> usedRegisterUAV{ 0 };

	collectBindingInfo(m_pso.m_VSState, usedRegisterCBV, usedRegisterSRV);
	collectBindingInfo(m_pso.m_PSState, usedRegisterCBV, usedRegisterSRV);
	collectBindingInfo(m_pso.m_CSState, usedRegisterCBV, usedRegisterSRV, usedRegisterUAV);

	const auto usedCBVCount = std::count_if(usedRegisterCBV.begin(), usedRegisterCBV.end(), [](u32 u)->bool { return u > 0; });
	if (usedCBVCount > 0)
		descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, (u32)usedCBVCount, 0));

	checkBindingInfo(usedRegisterSRV);
	const auto usedSRVCount = std::count(usedRegisterSRV.begin(), usedRegisterSRV.end(), 2U);
	if (usedSRVCount > 0)
		descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (u32)usedSRVCount, 0));

	checkBindingInfo(usedRegisterUAV);
	const auto usedUAVCount = std::count(usedRegisterUAV.begin(), usedRegisterUAV.end(), 2U);
	if (usedUAVCount > 0)
		descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, (u32)usedUAVCount, 0));

	m_pso.m_usedCBV_SRV_UAV_Count = static_cast<u32>(usedCBVCount + usedSRVCount + usedUAVCount);

	CD3DX12_ROOT_PARAMETER slotRootParameter;
	slotRootParameter.InitAsDescriptorTable((u32)descriptorRanges.size(), descriptorRanges.data());

	// A root signature is an array of root parameters.
	const u32 numParameters = m_pso.m_usedCBV_SRV_UAV_Count == 0 ? 0 : 1;
	auto samplers = ConfigStaticSamplerStates();
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(numParameters, &slotRootParameter, 
		static_cast<u32>(samplers.size()), samplers.empty() ? nullptr : samplers.data(),
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

bool DevicePipelineStateObjectDX12::IsEmptyRootParams() const
{
	return m_pso.m_usedCBV_SRV_UAV_Count == 0;
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

DeviceRTPipelineStateObjectDX12::DeviceRTPipelineStateObjectDX12(DeviceDX12* d, RTPipelineStateObject& rtPSO)
	: DeviceObject(nullptr)
	, m_rtPSO(rtPSO)
{
	BuildAccelerationStructures(d);
}

DeviceRTPipelineStateObjectDX12::~DeviceRTPipelineStateObjectDX12()
{

}

ID3D12StateObject* DeviceRTPipelineStateObjectDX12::GetDeviceRTPSO()
{
	assert(m_devicePSO.Get());
	return m_devicePSO.Get();
}

void DeviceRTPipelineStateObjectDX12::BuildAccelerationStructures(DeviceDX12* d)
{
	auto device = d->GetDevice();
	auto cmdListDevice = d->DeviceCommandList();
	auto& cmdList = d->GetCmdList();

	auto vertexBuffer = device_cast<DeviceResourceDX12*>(&m_rtPSO.m_geometry.front().first);
	auto indexBuffer = device_cast<DeviceResourceDX12*>(&m_rtPSO.m_geometry.front().second);

	cmdList.Reset();

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = indexBuffer->GetGPUAddress();
	geometryDesc.Triangles.IndexCount = m_rtPSO.m_geometry.front().second.GetNumElements();
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = m_rtPSO.m_geometry.front().first.GetNumElements();
	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = m_rtPSO.m_geometry.front().first.GetElementSize();

	// Mark the geometry as opaque. 
	// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
	// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// Get required sizes for an acceleration structure.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	assert(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;
	device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	assert(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	DeviceResCom12Ptr scratchResource = 
		DeviceBufferDX12::AllocateUAVBuffer(device, 
			std::max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	// Allocate resources for acceleration structures.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn’t need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		m_bottomLevelAccelerationStructure = DeviceBufferDX12::AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, initialResourceState, L"BottomLevelAccelerationStructure");
		m_topLevelAccelerationStructure = DeviceBufferDX12::AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, initialResourceState, L"TopLevelAccelerationStructure");
	}

	// Create an instance desc for the bottom-level acceleration structure.
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	DeviceResCom12Ptr instanceDescs = DeviceBufferDX12::AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), L"InstanceDescs");

	// Bottom Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	{
		bottomLevelBuildDesc.Inputs = bottomLevelInputs;
		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	}

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	{
		topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs = topLevelInputs;
		topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
	}

	auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
	{
		raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
		auto resource_barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get());
		raytracingCommandList->ResourceBarrier(1, &resource_barrier);
		raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	};

	// Build acceleration structure.
	BuildAccelerationStructure(cmdListDevice);

	d->GetDefaultQueue()->ExecuteCommandList();
	d->GetDefaultQueue()->Flush();
}

void DeviceRTPipelineStateObjectDX12::BuildRootSignature(DeviceDX12* d)
{
	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	{
		CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
		UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		CD3DX12_ROOT_PARAMETER rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &UAVDescriptor);
		rootParameters[1].InitAsShaderResourceView(0);
		CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);

		BlobComPtr serializedRootSig = nullptr;
		BlobComPtr errorBlob = nullptr;
		HR(D3D12SerializeRootSignature(&globalRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

		if (errorBlob != nullptr)
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());

		HR(d->GetDevice()->CreateRootSignature(0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_raytracingGlobalRootSignature)));
	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	{
		CD3DX12_ROOT_PARAMETER rootParameters[1];
		//rootParameters[0].InitAsConstants(SizeOfInUint32(m_rayGenCB), 0, 0);
		CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		BlobComPtr serializedRootSig = nullptr;
		BlobComPtr errorBlob = nullptr;
		HR(D3D12SerializeRootSignature(&localRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

		if (errorBlob != nullptr)
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());

		HR(d->GetDevice()->CreateRootSignature(0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_raytracingLocalRootSignature)));
	}
}