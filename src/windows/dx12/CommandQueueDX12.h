//***************************************************************************************
// CommandQueueDX12.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/CommandQueue.h"
#include "dx12Util.h"

namespace forward
{
	class CommandListDX12;
	class CommandQueueDX12 : public CommandQueue
	{
	public:
		virtual ~CommandQueueDX12();

		shared_ptr<CommandList> GetCommandList() override;
		u64 ExecuteCommandList() override;
		u64 Signal() override;
		bool IsFenceComplete(u64 fenceValue) override;
		void WaitForGPU(u64 fenceValue) override;
		void Flush() override;
		void WaitFor(const CommandQueue& other) override;

		shared_ptr<CommandListDX12> GetCommandListDX12();

	protected:
		CommandQueueDX12(Device& d, QueueType t);
		CommandQueueComPtr	m_CommandQueue;
		FenceComPtr					m_Fence;
		atomic_u64						m_FenceValue;

		friend class DeviceDX12;
	};
}