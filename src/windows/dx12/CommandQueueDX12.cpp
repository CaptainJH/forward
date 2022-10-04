//***************************************************************************************
// CommandQueueDX12.cpp by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#include "CommandQueueDX12.h"
#include "DeviceDX12.h"

using namespace forward;

CommandQueueDX12::CommandQueueDX12(Device& d, QueueType t)
	: CommandQueue(d, t)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (t == QueueType::Compute)
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	auto device = dynamic_cast<DeviceDX12*>(&m_device)->GetDevice();
	HR(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));
}

CommandQueueDX12::~CommandQueueDX12()
{}

shared_ptr<CommandList> CommandQueueDX12::GetCommandList()
{
	return make_shared<CommandListDX12>(m_device);
}

u64 CommandQueueDX12::ExecuteCommandList(shared_ptr<CommandList> /*commandList*/)
{
	return 0;
}

u64 CommandQueueDX12::Signal()
{
	return 0;
}

bool CommandQueueDX12::IsFenceComplete(u64 /*fenceValue*/)
{
	return false;
}

void CommandQueueDX12::WaitForFenceValue(u64 /*fenceValue*/)
{

}

void CommandQueueDX12::Flush()
{

}

void CommandQueueDX12::Wait(const CommandQueue& /*other*/)
{

}