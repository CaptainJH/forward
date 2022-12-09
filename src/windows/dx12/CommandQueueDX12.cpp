//***************************************************************************************
// CommandQueueDX12.cpp by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#include "CommandQueueDX12.h"
#include "CommandListDX12.h"
#include "DeviceDX12.h"

using namespace forward;

CommandQueueDX12::CommandQueueDX12(Device& d, QueueType t)
	: CommandQueue(d, t)
	, m_FenceValue(0)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (t == QueueType::Compute)
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	auto device = dynamic_cast<DeviceDX12*>(&m_device)->GetDevice();
	HR(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));
	HR(device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
}

CommandQueueDX12::~CommandQueueDX12()
{}

shared_ptr<CommandList> CommandQueueDX12::GetCommandList()
{
	if (m_CmdListPool.empty())
	{
		shared_ptr<CommandListDX12> ret = new CommandListDX12(m_device, m_queueType);
		m_CmdListPool.push_back(ret);
		return ret;
	}
	else
		return m_CmdListPool[0];
}

shared_ptr<CommandListDX12> CommandQueueDX12::GetCommandListDX12()
{
	auto ptr = GetCommandList();
	return shared_ptr<CommandListDX12>(static_cast<CommandListDX12*>(ptr.get()));
}

shared_ptr<CommandList> CommandQueueDX12::ExecuteCommandList()
{
	assert(!m_CmdListPool.empty());
	Flush();
	auto cmdListPtr = m_CmdListPool[0];
	cmdListPtr->Close();
	auto cmdList = static_cast<CommandListDX12*>(cmdListPtr.get());

	ID3D12CommandList* deviceCmdsLists[] = { cmdList->m_CmdList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(deviceCmdsLists), deviceCmdsLists);
	return cmdListPtr;
}

u64 CommandQueueDX12::Signal()
{
	u64 fenceValue = ++m_FenceValue;
	m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	return fenceValue;
}

bool CommandQueueDX12::IsFenceComplete(u64 fenceValue)
{
	return m_Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueueDX12::WaitForGPU(u64 fenceValue)
{
	// Wait until the GPU has completed commands up to this fence point.
	if (!IsFenceComplete(fenceValue))
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		HR(m_Fence->SetEventOnCompletion(fenceValue, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void CommandQueueDX12::Flush()
{
	if (!m_InFlightCmdLists.empty())
	{
		WaitForGPU(m_FenceValue);

		while (!m_InFlightCmdLists.empty())
		{
			CommandListEntry entry;
			while (!m_InFlightCmdLists.try_pop(entry))
			{
				auto cmdList = std::get<1>(entry);

				assert(IsFenceComplete(std::get<0>(entry)));
				cmdList->Reset();
				m_CmdListPool.push_back(cmdList);
			}
		}
	}
}

void CommandQueueDX12::WaitFor(const CommandQueue& other)
{
	const auto& otherQueue = static_cast<const CommandQueueDX12&>(other);
	m_CommandQueue->Wait(otherQueue.m_Fence.Get(), otherQueue.m_FenceValue);
}