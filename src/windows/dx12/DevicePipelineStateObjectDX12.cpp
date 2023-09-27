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

#include <ranges>
#include <regex>

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
			psoDesc.PrimitiveTopologyType = DevicePipelineStateObjectHelper::Convert2DX12TopologyType(pso.m_IAState.m_topologyType);
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
	BindingRanges rangesCBV;
	BindingRanges rangesSRV;
	BindingRanges rangesUAV;

	if (m_pso.m_VSState.m_shader)
	{
		auto deviceVS = device_cast<ShaderDX12*>(m_pso.m_VSState.m_shader);
		DevicePipelineStateObjectHelper::CollectBindingInfo(deviceVS, rangesCBV, rangesSRV, rangesUAV);
	}
	if (m_pso.m_PSState.m_shader)
	{
		auto devicePS = device_cast<ShaderDX12*>(m_pso.m_PSState.m_shader);
		DevicePipelineStateObjectHelper::CollectBindingInfo(devicePS, rangesCBV, rangesSRV, rangesUAV);
	}
	if (m_pso.m_CSState.m_shader)
	{
		auto deviceCS = device_cast<ShaderDX12*>(m_pso.m_CSState.m_shader);
		DevicePipelineStateObjectHelper::CollectBindingInfo(deviceCS, rangesCBV, rangesSRV, rangesUAV);
	}

	m_pso.m_usedCBV_SRV_UAV_Count = 0;
	for (auto& r : rangesCBV.m_ranges)
	{
		descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, r.Count(), r.bindStart));
		m_pso.m_usedCBV_SRV_UAV_Count += r.Count();
	}
	//DevicePipelineStateObjectHelper::CheckBindingResources(m_pso.m_VSState.m_constantBuffers, 
	//	m_pso.m_PSState.m_constantBuffers, rangesCBV, "CBV");

	for (auto& r : rangesSRV.m_ranges)
	{
		descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, r.Count(), r.bindStart));
		m_pso.m_usedCBV_SRV_UAV_Count += r.Count();
	}
	DevicePipelineStateObjectHelper::CheckBindingResources(m_pso.m_VSState.m_shaderResources, 
		m_pso.m_PSState.m_shaderResources, rangesSRV, "SRV");

	for (auto& r : rangesUAV.m_ranges)
	{
		descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, r.Count(), r.bindStart));
		m_pso.m_usedCBV_SRV_UAV_Count += r.Count();
	}
	DevicePipelineStateObjectHelper::CheckBindingResources(m_pso.m_CSState.m_uavShaderRes, rangesUAV, "UAV");

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

D3D12_PRIMITIVE_TOPOLOGY_TYPE DevicePipelineStateObjectHelper::Convert2DX12TopologyType(PrimitiveTopologyType topo)
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
	desc.FillMode = DevicePipelineStateObjectHelper::msFillMode[rsState.fillMode];
	desc.CullMode = DevicePipelineStateObjectHelper::msCullMode[rsState.cullMode];
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
		out.SrcBlend = DevicePipelineStateObjectHelper::msBlendMode[in.srcColor];
		out.DestBlend = DevicePipelineStateObjectHelper::msBlendMode[in.dstColor];
		out.BlendOp = DevicePipelineStateObjectHelper::msBlendOp[in.opColor];
		out.SrcBlendAlpha = DevicePipelineStateObjectHelper::msBlendMode[in.srcAlpha];
		out.DestBlendAlpha = DevicePipelineStateObjectHelper::msBlendMode[in.dstAlpha];
		out.BlendOpAlpha = DevicePipelineStateObjectHelper::msBlendOp[in.opAlpha];
		out.RenderTargetWriteMask = in.mask;
	}
}

void DevicePipelineStateObjectDX12::ConfigDepthStencilState(D3D12_DEPTH_STENCIL_DESC& desc) const
{
	desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	auto& dsState = m_pso.m_OMState.m_dsState;
	desc.DepthEnable = dsState.depthEnable ? TRUE : FALSE;
	desc.DepthWriteMask = DevicePipelineStateObjectHelper::msWriteMask[dsState.writeMask];
	desc.DepthFunc = DevicePipelineStateObjectHelper::msComparison[dsState.comparison];
	desc.StencilEnable = dsState.stencilEnable ? TRUE : FALSE;
	desc.StencilReadMask = dsState.stencilReadMask;
	desc.StencilWriteMask = dsState.stencilWriteMask;
	DepthStencilState::Face front = dsState.frontFace;
	desc.FrontFace.StencilFailOp = DevicePipelineStateObjectHelper::msStencilOp[front.fail];
	desc.FrontFace.StencilDepthFailOp = DevicePipelineStateObjectHelper::msStencilOp[front.depthFail];
	desc.FrontFace.StencilPassOp = DevicePipelineStateObjectHelper::msStencilOp[front.pass];
	desc.FrontFace.StencilFunc = DevicePipelineStateObjectHelper::msComparison[front.comparison];
	DepthStencilState::Face back = dsState.backFace;
	desc.BackFace.StencilFailOp = DevicePipelineStateObjectHelper::msStencilOp[back.fail];
	desc.BackFace.StencilDepthFailOp = DevicePipelineStateObjectHelper::msStencilOp[back.depthFail];
	desc.BackFace.StencilPassOp = DevicePipelineStateObjectHelper::msStencilOp[back.pass];
	desc.BackFace.StencilFunc = DevicePipelineStateObjectHelper::msComparison[back.comparison];
}

std::vector<CD3DX12_STATIC_SAMPLER_DESC> DevicePipelineStateObjectDX12::ConfigStaticSamplerStates() const
{
	return DevicePipelineStateObjectHelper::ConfigStaticSamplerStates(m_pso.m_PSState.m_samplers);
}

bool DevicePipelineStateObjectDX12::IsEmptyRootParams() const
{
	return m_pso.m_usedCBV_SRV_UAV_Count == 0;
}

D3D12_FILL_MODE const DevicePipelineStateObjectHelper::msFillMode[] =
{
	D3D12_FILL_MODE_SOLID,
	D3D12_FILL_MODE_WIREFRAME
};

D3D12_CULL_MODE const DevicePipelineStateObjectHelper::msCullMode[] =
{
	D3D12_CULL_MODE_NONE,
	D3D12_CULL_MODE_FRONT,
	D3D12_CULL_MODE_BACK
};

D3D12_BLEND const DevicePipelineStateObjectHelper::msBlendMode[] =
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

D3D12_BLEND_OP const DevicePipelineStateObjectHelper::msBlendOp[] =
{
	D3D12_BLEND_OP_ADD,
	D3D12_BLEND_OP_SUBTRACT,
	D3D12_BLEND_OP_REV_SUBTRACT,
	D3D12_BLEND_OP_MIN,
	D3D12_BLEND_OP_MAX,
};

D3D12_DEPTH_WRITE_MASK const DevicePipelineStateObjectHelper::msWriteMask[] =
{
	D3D12_DEPTH_WRITE_MASK_ZERO,
	D3D12_DEPTH_WRITE_MASK_ALL
};

D3D12_COMPARISON_FUNC const DevicePipelineStateObjectHelper::msComparison[] =
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

D3D12_STENCIL_OP const DevicePipelineStateObjectHelper::msStencilOp[] =
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

D3D12_FILTER const DevicePipelineStateObjectHelper::msFilter[] =
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

D3D12_TEXTURE_ADDRESS_MODE const DevicePipelineStateObjectHelper::msAddressMode[] =
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
	BuildRaytracingPipelineStateObject(d);
	BuildAccelerationStructures(d);
	BuildShaderTables(d);
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

	auto BuildAccelerationStructure = [&](auto& buildDesc, ID3D12Resource* res) {
		cmdListDevice->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
		auto resource_barrier = CD3DX12_RESOURCE_BARRIER::UAV(res);
		cmdListDevice->ResourceBarrier(1, &resource_barrier);
		};
	const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags 
		= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	Vector<DeviceResCom12Ptr> blasScrachResources;
	for (auto& geo : m_rtPSO.m_meshes)
	{
		auto vertexBuffer = device_cast<DeviceResourceDX12*>(geo.first);
		auto indexBuffer = device_cast<DeviceResourceDX12*>(geo.second);
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
			// Mark the geometry as opaque. 
			// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
			// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
			.Triangles = {
				.Transform3x4 = 0,
				.IndexFormat = DXGI_FORMAT_R32_UINT,
				.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
				.IndexCount = geo.second->GetNumElements(),
				.VertexCount = geo.first->GetNumElements(),
				.IndexBuffer = indexBuffer->GetGPUAddress(),
				.VertexBuffer = {
					.StartAddress = vertexBuffer->GetGPUAddress(),
					.StrideInBytes = geo.first->GetElementSize(),
				}
				}
			};

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
			.Flags = buildFlags,
			.NumDescs = 1,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = &geometryDesc
		};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
		device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
		assert(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);
		bottomLevelPrebuildInfo.ScratchDataSizeInBytes = 
			Align<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT>(bottomLevelPrebuildInfo.ScratchDataSizeInBytes);
		bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes = 
			Align<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT>(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes);

		auto scratchResource = blasScrachResources.emplace_back(
			DeviceBufferDX12::AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes,
				D3D12_RESOURCE_STATE_COMMON, L"ScratchResource"));

		auto& blas = m_bottomLevelAccelerationStructures.emplace_back(DeviceBufferDX12::AllocateUAVBuffer(device, 
			bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, 
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"BottomLevelAccelerationStructure"));

		// Bottom Level Acceleration Structure desc
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
			.DestAccelerationStructureData = blas->GetGPUVirtualAddress(),
			.Inputs = bottomLevelInputs,
			.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress(),
		};
		BuildAccelerationStructure(bottomLevelBuildDesc, blas.Get());
	}

	// Get required sizes for an acceleration structure.
	// Functions for encoding/decoding material and geometry ID into single integer
	auto packInstanceID = [](u32 materialID, u32 geometryID)->u32 {
		return ((geometryID & 0x3FFF) << 10) | (materialID & 0x3FF);
		};
	Vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;
	for (auto& ins : m_rtPSO.m_instances)
		instances.emplace_back(D3D12_RAYTRACING_INSTANCE_DESC{
			.Transform = {
				ins.object2WorldMat.x[0][0], ins.object2WorldMat.x[1][0], ins.object2WorldMat[2][0], ins.object2WorldMat[3][0],
				ins.object2WorldMat.x[0][1], ins.object2WorldMat.x[1][1], ins.object2WorldMat[2][1], ins.object2WorldMat[3][1],
				ins.object2WorldMat.x[0][2], ins.object2WorldMat.x[1][2], ins.object2WorldMat[2][2], ins.object2WorldMat[3][2],
			},
			.InstanceID = packInstanceID(ins.materialId, ins.meshId),
			.InstanceMask = 0xFF,
			.InstanceContributionToHitGroupIndex = 0,
			.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE | D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE,
			.AccelerationStructure = m_bottomLevelAccelerationStructures[ins.meshId]->GetGPUVirtualAddress()
			});
	if (instances.empty())
		instances.emplace_back(D3D12_RAYTRACING_INSTANCE_DESC{
			.Transform = {
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
			},
			.InstanceID = 0,
			.InstanceMask = 0xFF,
			.InstanceContributionToHitGroupIndex = 0,
			.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE | D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE,
			.AccelerationStructure = m_bottomLevelAccelerationStructures.front()->GetGPUVirtualAddress()
			});
	DeviceResCom12Ptr instanceDescBuffer = 
		DeviceBufferDX12::AllocateUploadBuffer(device, instances.data(), instances.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), L"InstanceDescs");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
		.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
		.Flags = buildFlags,
		.NumDescs = static_cast<u32>(instances.size()),
		.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
		.InstanceDescs = instanceDescBuffer->GetGPUVirtualAddress()
	};

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	assert(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);
	topLevelPrebuildInfo.ScratchDataSizeInBytes = 
		Align<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT>(topLevelPrebuildInfo.ScratchDataSizeInBytes);
	topLevelPrebuildInfo.ResultDataMaxSizeInBytes = 
		Align<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	// Allocate resources for acceleration structures.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn't need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
	DeviceResCom12Ptr scratchResource = 
		DeviceBufferDX12::AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes,
			D3D12_RESOURCE_STATE_COMMON, L"ScratchResource");

	m_topLevelAccelerationStructure = DeviceBufferDX12::AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, 
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {
		.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress(),
		.Inputs = topLevelInputs,
		.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress()
	};

	BuildAccelerationStructure(topLevelBuildDesc, m_topLevelAccelerationStructure.Get());

	d->GetDefaultQueue()->ExecuteCommandList([]() {});
	d->GetDefaultQueue()->Flush();
}

void DeviceRTPipelineStateObjectDX12::BuildRootSignature(DeviceDX12* d)
{
	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	{
		std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;

		const auto MaxDescriptor = 256;
		std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;
		descriptorRanges.reserve(MaxDescriptor);
		auto deviceShader = device_cast<ShaderDX12*>(m_rtPSO.m_rtState.m_shader);
		m_rtPSO.m_usedCBV_SRV_UAV_Count = 0;

		auto GenerateDescriptorRanges = [&](u32 space, auto& stage) {

			BindingRanges rangesCBV;
			BindingRanges rangesSRV;
			BindingRanges rangesUAV;

			DevicePipelineStateObjectHelper::CollectBindingInfo(deviceShader, rangesCBV, rangesSRV, rangesUAV, space);

			for (auto& r : rangesCBV.m_ranges)
			{
				descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, r.Count(), r.bindStart, space));
				m_rtPSO.m_usedCBV_SRV_UAV_Count += r.Count();
			}

			for (auto& r : rangesSRV.m_ranges)
			{
				descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, r.Count(), r.bindStart, space));
				m_rtPSO.m_usedCBV_SRV_UAV_Count += r.Count();
			}

			for (auto& r : rangesUAV.m_ranges)
			{
				descriptorRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, r.Count(), r.bindStart, space));
				m_rtPSO.m_usedCBV_SRV_UAV_Count += r.Count();
			}
			DevicePipelineStateObjectHelper::CheckBindingResources(stage.m_constantBuffers, rangesCBV, "CBV", space);
			DevicePipelineStateObjectHelper::CheckBindingResources(stage.m_shaderResources, rangesSRV, "SRV", space);
			DevicePipelineStateObjectHelper::CheckBindingResources(stage.m_uavShaderRes, rangesUAV, "UAV", space);
			};

		GenerateDescriptorRanges(0, m_rtPSO.m_rtState);
		std::ranges::sort(m_rtPSO.m_rtState.m_bindlessShaderStageStates, {}, [](auto& s) { return s.m_space; });
		for (auto& stage : m_rtPSO.m_rtState.m_bindlessShaderStageStates)
			GenerateDescriptorRanges(stage.m_space, stage);

		if (!descriptorRanges.empty())
		{
			CD3DX12_ROOT_PARAMETER cbv_srv_uav_root_parameter;
			cbv_srv_uav_root_parameter.InitAsDescriptorTable((u32)descriptorRanges.size(), descriptorRanges.data());
			slotRootParameters.push_back(cbv_srv_uav_root_parameter);
		}

		CD3DX12_ROOT_PARAMETER scene_root_parameter;
		scene_root_parameter.InitAsShaderResourceView(0, ShaderDX12::AccelerationStructuresSpace);
		slotRootParameters.push_back(scene_root_parameter);

		BindingRanges samplerRange;
		DevicePipelineStateObjectHelper::CollectSamplerInfo(deviceShader, samplerRange);
		DevicePipelineStateObjectHelper::CheckBindingResources(m_rtPSO.m_rtState.m_samplers, samplerRange, "Sampler");
		auto samplers = ConfigStaticSamplerStates();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			(u32)slotRootParameters.size(), slotRootParameters.data(),
			static_cast<u32>(samplers.size()), samplers.empty() ? nullptr : samplers.data());

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HR(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		HR(d->GetDevice()->CreateRootSignature(0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_raytracingGlobalRootSignature)));
	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	{
		CD3DX12_ROOT_PARAMETER rootParameters[1];
		const auto n = ( m_rtPSO.m_rtState.m_rayGenShaderTable->m_shaderRecords.front().shaderArguments.size()
			+ m_rtPSO.m_rtState.m_hitShaderTable->m_shaderRecords.front().shaderArguments.size()
			) / sizeof(u32);
		rootParameters[0].InitAsConstants(static_cast<u32>(n), 1);
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

void DeviceRTPipelineStateObjectDX12::BuildRaytracingPipelineStateObject(DeviceDX12* device)
{
	// Create 7 subobjects that combine into a RTPSO:
	// Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
	// Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
	// This simple sample utilizes default shader association except for local root signature subobject
	// which has an explicit association specified purely for demonstration purposes.
	// 1 - DXIL library
	// 1 - Triangle hit group
	// 1 - Shader config
	// 2 - Local root signature and association
	// 1 - Global root signature
	// 1 - Pipeline config
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };


	// DXIL library
	// This contains the shaders and their entrypoints for the state object.
	// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
	auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	auto rtShader = m_rtPSO.m_rtState.m_shader;
	forward::shared_ptr<ShaderDX12> deviceRT = device_cast<ShaderDX12*>(rtShader);
	if (rtShader && !deviceRT)
	{
		deviceRT = forward::make_shared<ShaderDX12>(rtShader.get());
		rtShader->SetDeviceObject(deviceRT);
	}
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(deviceRT->GetCompiledCode(), deviceRT->GetCompiledCodeSize());
	lib->SetDXILLibrary(&libdxil);
	BuildRootSignature(device);
	// Define which shader exports to surface from the library.
	// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
	// In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
	for (auto& s : m_rtPSO.m_rtState.m_hitShaderTable->m_shaderRecords)
		lib->DefineExport(s.shaderName.c_str());
	for (auto& s : m_rtPSO.m_rtState.m_rayGenShaderTable->m_shaderRecords)
		lib->DefineExport(s.shaderName.c_str());
	for (auto& s : m_rtPSO.m_rtState.m_missShaderTable->m_shaderRecords)
		lib->DefineExport(s.shaderName.c_str());

	// Triangle hit group
	// A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
	// In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
	auto hitGroups = GetHitGroupsInfo();
	for (auto& p : hitGroups)
	{
		auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		auto closestHitRegex = std::wregex{ L"^.*closesthit.*$", std::regex_constants::icase };
		auto anyHitRegex = std::wregex{ L"^.*anyhit.*$", std::regex_constants::icase };
		for (auto& s : p.second)
		{
			if (std::regex_match(s, closestHitRegex))
				hitGroup->SetClosestHitShaderImport(s.c_str());
			else if (std::regex_match(s, anyHitRegex))
				hitGroup->SetAnyHitShaderImport(s.c_str());
			else
				assert(false);
		}
		hitGroup->SetHitGroupExport(p.first.c_str());
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}

	// Shader config
	// Defines the maximum sizes in bytes for the ray payload and attribute structure.
	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	shaderConfig->Config(m_rtPSO.m_maxPayloadSizeInByte, D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES);

	// Local root signature and shader association
	//CreateLocalRootSignatureSubobjects(&raytracingPipeline);
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.

	// Global root signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

	// Pipeline config
	// Defines the maximum TraceRay() recursion depth.
	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	// PERFOMANCE TIP: Set max recursion depth as low as needed 
	// as drivers may apply optimization strategies for low recursion depths. 
	UINT maxRecursionDepth = 1; // ~ primary rays only. 
	pipelineConfig->Config(maxRecursionDepth);

#if _DEBUG
	PrintStateObjectDesc(raytracingPipeline);
#endif

	// Create the state object.
	HR(device->GetDevice()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_devicePSO)));
}

void DeviceRTPipelineStateObjectDX12::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
	// Hit group and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

	// Local root signature to be used in a ray gen shader.
	{
		auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());
		// Shader association
		if (m_rtPSO.m_rtState.m_rayGenShaderTable->ContainPayload())
		{
			auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
			rootSignatureAssociation->AddExport(m_rtPSO.m_rtState.m_rayGenShaderTable->m_shaderRecords.front().shaderName.c_str());
		}
		if (m_rtPSO.m_rtState.m_hitShaderTable->ContainPayload())
		{
			auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
			const WString hitGroupName = TextHelper::ToUnicode(m_rtPSO.m_rtState.m_hitShaderTable->Name());
			rootSignatureAssociation->AddExport(hitGroupName.c_str());
		}
	}
}

void DeviceRTPipelineStateObjectDX12::BuildShaderTables(DeviceDX12* d)
{
	std::unordered_map<WString, void*> shaderName2ShaderIdentifierTable;
	auto hitGroups = GetHitGroupsInfo();

	auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
	{
			for (auto& record : m_rtPSO.m_rtState.m_rayGenShaderTable->m_shaderRecords)
			{
				shaderName2ShaderIdentifierTable[record.shaderName] = 
					stateObjectProperties->GetShaderIdentifier(record.shaderName.c_str());
			}
			for (auto& record : m_rtPSO.m_rtState.m_missShaderTable->m_shaderRecords)
			{
				shaderName2ShaderIdentifierTable[record.shaderName] =
					stateObjectProperties->GetShaderIdentifier(record.shaderName.c_str());
			}
			for (auto& p : hitGroups)
				shaderName2ShaderIdentifierTable[p.first] = stateObjectProperties->GetShaderIdentifier(p.first.c_str());
	};

	// Get shader identifiers.
	UINT shaderIdentifierSize;
	{
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
		HR(m_devicePSO.As(&stateObjectProperties));
		GetShaderIdentifiers(stateObjectProperties.Get());
		shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	}


	auto CreateDeviceShaderTable = [&](shared_ptr<ShaderTable> st) {
		if (st && !st->DeviceObject())
		{
			st->SetupShaderRecords(shaderName2ShaderIdentifierTable);
			auto deviceShaderTable = make_shared<DeviceBufferDX12>(nullptr, st.get(), *d);
			deviceShaderTable->SyncCPUToGPU();
			st->SetDeviceObject(deviceShaderTable);
		}
	};

	CreateDeviceShaderTable(m_rtPSO.m_rtState.m_rayGenShaderTable);
	CreateDeviceShaderTable(m_rtPSO.m_rtState.m_missShaderTable);
	CreateDeviceShaderTable(m_rtPSO.m_rtState.m_hitShaderTable);
}

std::unordered_map<WString, Vector<WString>> DeviceRTPipelineStateObjectDX12::GetHitGroupsInfo() const
{
	std::unordered_map<WString, Vector<WString>> ret;
	for (auto& s : m_rtPSO.m_rtState.m_hitShaderTable->m_shaderRecords)
	{
		const auto sepIt = s.shaderName.find(L"_");
		if (sepIt != WString::npos)
			ret[s.shaderName.substr(0, sepIt)].push_back(s.shaderName);
	}

	return ret;
}

// Pretty-print a state object tree.
void DeviceRTPipelineStateObjectDX12::PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
	std::wstringstream wstr;
	wstr << L"\n";
	wstr << L"--------------------------------------------------------------------\n";
	wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

	auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
		{
			std::wostringstream woss;
			for (UINT i = 0; i < numExports; i++)
			{
				woss << L"|";
				if (depth > 0)
				{
					for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
				}
				woss << L" [" << i << L"]: ";
				if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
				woss << exports[i].Name << L"\n";
			}
			return woss.str();
		};

	for (UINT i = 0; i < desc->NumSubobjects; i++)
	{
		wstr << L"| [" << i << L"]: ";
		switch (desc->pSubobjects[i].Type)
		{
		case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
			wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
			wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
			wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
		{
			wstr << L"DXIL Library 0x";
			auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
			wstr << ExportTree(1, lib->NumExports, lib->pExports);
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
		{
			wstr << L"Existing Library 0x";
			auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << collection->pExistingCollection << L"\n";
			wstr << ExportTree(1, collection->NumExports, collection->pExports);
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
		{
			wstr << L"Subobject to Exports Association (Subobject [";
			auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
			UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
			wstr << index << L"])\n";
			for (UINT j = 0; j < association->NumExports; j++)
			{
				wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
			}
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
		{
			wstr << L"DXIL Subobjects to Exports Association (";
			auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
			wstr << association->SubobjectToAssociate << L")\n";
			for (UINT j = 0; j < association->NumExports; j++)
			{
				wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
			}
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
		{
			wstr << L"Raytracing Shader Config\n";
			auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
			wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
		{
			wstr << L"Raytracing Pipeline Config\n";
			auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
		{
			wstr << L"Hit Group (";
			auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
			wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
			wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
			wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
			break;
		}
		}
		wstr << L"|--------------------------------------------------------------------\n";
	}
	wstr << L"\n";
	OutputDebugStringW(wstr.str().c_str());
}

std::vector<CD3DX12_STATIC_SAMPLER_DESC> DeviceRTPipelineStateObjectDX12::ConfigStaticSamplerStates() const
{
	return DevicePipelineStateObjectHelper::ConfigStaticSamplerStates(m_rtPSO.m_rtState.m_samplers);
}

void DevicePipelineStateObjectHelper::CollectBindingInfo(const ShaderDX12* deviceShader, BindingRanges& rangesCBV, BindingRanges& rangesSRV, BindingRanges& rangesUAV, u32 space)
{
	if (!deviceShader)
		return;

	for (auto& cb : deviceShader->GetCBuffers())
	{
		if (cb.GetSpace() != space) continue;
		auto register_index = cb.GetBindPoint();
		BindingRange range = { register_index, register_index + cb.GetBindCount() - 1 };
		rangesCBV.AddRange(range);
	}

	for (auto& tex : deviceShader->GetTextures())
	{
		if (tex.IsGpuWritable() || tex.GetSpace() != space) continue;
		auto register_index = tex.GetBindPoint();
		BindingRange range = { register_index, register_index + tex.GetBindCount() - 1 };
		rangesSRV.AddRange(range);
	}
	for (auto& tex : deviceShader->GetTextureArrays())
	{
		if (tex.IsGpuWritable() || tex.GetSpace() != space) continue;
		auto register_index = tex.GetBindPoint();
		BindingRange range = { register_index, register_index + tex.GetBindCount() - 1 };
		rangesSRV.AddRange(range);
	}
	for (auto& buf : deviceShader->GetByteAddressBuffers())
	{
		if (buf.IsGpuWritable() || buf.GetSpace() != space) continue;
		auto register_index = buf.GetBindPoint();
		BindingRange range = { register_index, register_index + buf.GetBindCount() - 1 };
		rangesSRV.AddRange(range);
	}
	for (auto& buf : deviceShader->GetStructuredBuffers())
	{
		if (buf.IsGpuWritable() || buf.GetSpace() != space) continue;
		auto register_index = buf.GetBindPoint();
		BindingRange range = { register_index, register_index + buf.GetBindCount() - 1 };
		rangesSRV.AddRange(range);
	}

	for (auto& tex : deviceShader->GetTextures())
	{
		if (tex.IsGpuWritable() && tex.GetSpace() == space)
		{
			auto register_index = tex.GetBindPoint();
			BindingRange range = { register_index, register_index + tex.GetBindCount() - 1 };
			rangesUAV.AddRange(range);
		}
	}

	std::ranges::sort(rangesCBV.m_ranges, {}, [](auto& r) { return r.bindStart; });
	std::ranges::sort(rangesSRV.m_ranges, {}, [](auto& r) { return r.bindStart; });
	std::ranges::sort(rangesUAV.m_ranges, {}, [](auto& r) { return r.bindStart; });
}

void DevicePipelineStateObjectHelper::CollectSamplerInfo(const ShaderDX12* deviceShader, BindingRanges& ranges)
{
	if (!deviceShader)
		return;

	for (auto& s : deviceShader->GetSamplers())
	{
		auto register_index = s.GetBindPoint();
		BindingRange range = { register_index, register_index + s.GetBindCount() - 1 };
		ranges.AddRange(range);
	}
	std::ranges::sort(ranges.m_ranges, {}, [](auto& r) { return r.bindStart; });
}