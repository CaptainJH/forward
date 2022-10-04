//***************************************************************************************
// CommandQueueDX12.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/CommandQueue.h"
#include "CommandListDX12.h"

namespace forward
{
	class CommandQueueDX12 : public CommandQueue
	{
	public:
		virtual ~CommandQueueDX12();

		shared_ptr<CommandList> GetCommandList() override;
		u64 ExecuteCommandList(shared_ptr<CommandList> commandList) override;
		u64 Signal() override;
		bool IsFenceComplete(u64 fenceValue) override;
		void WaitForFenceValue(u64 fenceValue) override;
		void Flush() override;
		void Wait(const CommandQueue& other) override;

	protected:
		CommandQueueDX12(Device& d, QueueType t);
		CommandQueueComPtr	m_CommandQueue;

		friend class DeviceDX12;
	};
}