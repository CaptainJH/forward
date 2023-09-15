//***************************************************************************************
// CommandListDX12.cpp by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#include "CommandListDX12.h"
#include "RHI/CommandQueue.h"
#include "ResourceSystem/DeviceBufferDX12.h"
#include "ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DevicePipelineStateObjectDX12.h"
#include "DeviceDX12.h"

using namespace forward;

void CommandListDX12::Reset()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	HR(m_CmdListAlloc->Reset());
	HR(m_CmdList->Reset(m_CmdListAlloc.Get(), nullptr));

	std::for_each(std::begin(m_DynamicDescriptorHeaps), std::end(m_DynamicDescriptorHeaps),
		[](DynamicDescriptorHeapDX12& heap) {
			heap.Reset();
		});
}

void CommandListDX12::Close()
{
	HR(m_CmdList->Close());
}

CommandListDX12::CommandListDX12(Device& d, QueueType t) 
	: CommandList(d) 
{
	auto& device = static_cast<DeviceDX12&>(m_device);
	const auto cmdListType = t == QueueType::Direct 
		? D3D12_COMMAND_LIST_TYPE_DIRECT
		: D3D12_COMMAND_LIST_TYPE_COMPUTE;;

	HR(device.GetDevice()->CreateCommandAllocator(cmdListType,
		IID_PPV_ARGS(m_CmdListAlloc.GetAddressOf())));

	HR(device.GetDevice()->CreateCommandList(0, cmdListType,
		m_CmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_CmdList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	HR(m_CmdList->Close());
}

CommandListDX12::~CommandListDX12() 
{}

void CommandListDX12::DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color)
{
	msg; x; y; color;

}

void CommandListDX12::Draw(u32 vertexNum, u32 startVertexLocation)
{
	m_CmdList->DrawInstanced(vertexNum, 1, startVertexLocation, 0);
}

void CommandListDX12::DrawIndexed(u32 indexCount)
{
	m_CmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void CommandListDX12::Dispatch(u32 x, u32 y, u32 z)
{
	m_CmdList->Dispatch(x, y, z);
}

void CommandListDX12::BeginDrawFrameGraph(FrameGraph* fg)
{
	fg;
}

void CommandListDX12::EndDrawFrameGraph()
{

}

void CommandListDX12::DrawRenderPass(RenderPass& pass)
{
	pass;
}

void CommandListDX12::PopulateCmdsFrom(FrameGraph*)
{

}

void CommandListDX12::CommitStagedDescriptors()
{
	for (auto& heap : m_DynamicDescriptorHeaps)
		heap.CommitStagedDescriptors(GetDeviceDX12());
}

void CommandListDX12::BindGPUVisibleHeaps()
{
	auto& cbvHeap = m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	cbvHeap.BindGPUVisibleDescriptorHeap(*this);
}

void CommandListDX12::PrepareGPUVisibleHeaps(RenderPass& pass)
{
	auto& pso = pass.GetPSO();

	if (pso.m_usedCBV_SRV_UAV_Count > 0)
	{
		auto& heap = m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
		if (auto baseDescriptorHandleAddr = heap.PrepareDescriptorHandleCache(pso.m_usedCBV_SRV_UAV_Count))
		{
			u32 stagedCBVs = 0;
			u32 stagedSRVs = 0;
			u32 stagedUAVs = 0;
			auto stageCBVFunc = [&](DeviceBufferDX12* deviceCB) {
				assert(deviceCB);
				*(baseDescriptorHandleAddr + stagedCBVs++) = deviceCB->GetCBViewCPUHandle();
			};
			auto stageSRVFunc = [&](DeviceTextureDX12* deviceTex) {
				assert(deviceTex);
				*(baseDescriptorHandleAddr + stagedCBVs + stagedSRVs++) = deviceTex->GetShaderResourceViewHandle();
			};
			auto stageUAVFunc = [&](DeviceTextureDX12* deviceTex) {
				assert(deviceTex);
				*(baseDescriptorHandleAddr + stagedCBVs + stagedSRVs + stagedUAVs++) = deviceTex->GetUnorderedAccessViewHandle();
			};

			// stage CBVs
			for (auto i = 0; i < FORWARD_RENDERER_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; ++i)
			{
				if (auto cb_vs = pso.m_VSState.m_constantBuffers[i])
				{
					auto deviceCB = device_cast<DeviceBufferDX12*>(cb_vs);
					stageCBVFunc(deviceCB);
				}
				else if (auto cb_gs = pso.m_GSState.m_constantBuffers[i])
				{
					auto deviceCB = device_cast<DeviceBufferDX12*>(cb_gs);
					stageCBVFunc(deviceCB);
				}
				else if (auto cb_ps = pso.m_PSState.m_constantBuffers[i])
				{
					auto deviceCB = device_cast<DeviceBufferDX12*>(cb_ps);
					stageCBVFunc(deviceCB);
				}
				else
					break;
			}

			// stage SRVs
			for (auto i = 0; i < FORWARD_RENDERER_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i)
			{
				if (auto res_vs = pso.m_VSState.m_shaderResources[i])
				{
					auto deviceTex = device_cast<DeviceTexture2DDX12*>(res_vs);
					stageSRVFunc(deviceTex);
				}
				else if (auto res_gs = pso.m_GSState.m_shaderResources[i])
				{
					auto deviceTex = device_cast<DeviceTexture2DDX12*>(res_gs);
					stageSRVFunc(deviceTex);
				}
				else if (auto res_ps = pso.m_PSState.m_shaderResources[i])
				{
					auto deviceTex = device_cast<DeviceTexture2DDX12*>(res_ps);
					stageSRVFunc(deviceTex);
				}
				else
					break;
			}

			// stage UAVs
			for (auto i = 0; i < 8; ++i)
			{
				if (auto res_cs = pso.m_CSState.m_uavShaderRes[i])
				{
					auto deviceTex = device_cast<DeviceTexture2DDX12*>(res_cs);
					stageUAVFunc(deviceTex);
				}
				else
					break;
			}

			assert(stagedCBVs + stagedSRVs + stagedUAVs == pso.m_usedCBV_SRV_UAV_Count);
		}
	}

	if (pso.m_usedSampler_Count > 0)
	{
		auto& heap = m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER];
		heap.PrepareDescriptorHandleCache(pso.m_usedSampler_Count);

		// TODO: stage Samplers
	}
}

void CommandListDX12::PrepareGPUVisibleHeaps(RTPipelineStateObject& pso)
{
	auto& heap = m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	if (auto baseDescriptorHandleAddr = heap.PrepareDescriptorHandleCache(1))
	{
		u32 stagedUAVs = 0;

		auto stageUAVFunc = [&](DeviceTextureDX12* deviceTex) {
			assert(deviceTex);
			*(baseDescriptorHandleAddr + stagedUAVs++) = deviceTex->GetUnorderedAccessViewHandle();
			};

		// stage UAVs
		for (auto i = 0; i < 8; ++i)
		{
			if (auto res_cs = pso.m_rtState.m_uavShaderRes[i])
			{
				auto deviceTex = device_cast<DeviceTexture2DDX12*>(res_cs);
				stageUAVFunc(deviceTex);
			}
			else
				break;
		}
	}
}

DeviceDX12& CommandListDX12::GetDeviceDX12()
{
	return static_cast<DeviceDX12&>(m_device);
}

void CommandListDX12::SetDynamicConstantBuffer(ConstantBufferBase* cb)
{
	const auto u = reinterpret_cast<uintptr_t>(this);
	cb->ResetDeviceBuffer(u);
	cb->FetchDeviceBuffer(u, [&]()->ResourcePtr {
		auto deviceCB = forward::make_shared<DeviceBufferDX12>(m_CmdList.Get(), cb, GetDeviceDX12());
		return deviceCB;
		})->SyncCPUToGPU();
}

void CommandListDX12::BindGraphicsPSO(DevicePipelineStateObjectDX12& devicePSO)
{
	m_CmdList->SetGraphicsRootSignature(devicePSO.m_rootSignature.Get());
	m_CmdList->SetPipelineState(devicePSO.GetDevicePSO());
	for (auto i = 0U; i < devicePSO.m_pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		if (devicePSO.m_pso.m_IAState.m_vertexBuffers[i])
		{
			auto vbv = device_cast<DeviceBufferDX12*>(devicePSO.m_pso.m_IAState.m_vertexBuffers[i])->VertexBufferView();
			m_CmdList->IASetVertexBuffers(i, 1, &vbv);
		}
	}
	if (devicePSO.m_pso.m_IAState.m_indexBuffer)
	{
		auto ibv = device_cast<DeviceBufferDX12*>(devicePSO.m_pso.m_IAState.m_indexBuffer)->IndexBufferView();
		m_CmdList->IASetIndexBuffer(&ibv);
	}
	m_CmdList->IASetPrimitiveTopology(Convert2D3DTopology(devicePSO.m_pso.m_IAState.m_topologyType));

	if (!devicePSO.IsEmptyRootParams())
		for (auto& heap : m_DynamicDescriptorHeaps)
			heap.BindDescriptorTableToRootParam(m_CmdList.Get(), &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void CommandListDX12::BindComputePSO(DevicePipelineStateObjectDX12& devicePSO)
{
	m_CmdList->SetComputeRootSignature(devicePSO.m_rootSignature.Get());
	m_CmdList->SetPipelineState(devicePSO.GetDevicePSO());

	if (!devicePSO.IsEmptyRootParams())
		for (auto& heap : m_DynamicDescriptorHeaps)
			heap.BindDescriptorTableToRootParam(m_CmdList.Get(), &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

void CommandListDX12::BindRTPSO(DeviceRTPipelineStateObjectDX12& deviceRTPSO)
{
	m_CmdList->SetComputeRootSignature(deviceRTPSO.m_raytracingGlobalRootSignature.Get());
	m_CmdList->SetPipelineState1(deviceRTPSO.m_devicePSO.Get());

	for (auto& heap : m_DynamicDescriptorHeaps)
		heap.BindDescriptorTableToRootParam(m_CmdList.Get(), &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}