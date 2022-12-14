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
		void ExecuteCommandList() override;
		u64 Signal() override;
		bool IsFenceComplete(u64 fenceValue) override;
		void WaitForGPU(u64 fenceValue) override;
		void Flush() override;
		void WaitFor(const CommandQueue& other) override;

		shared_ptr<CommandListDX12> GetCommandListDX12();

		template<class F>
		u64 ExecuteCommandList(F&& func)
		{
			ExecuteCommandList();
			func();
			auto fenceValue = Signal();
			m_InFlightCmdLists.push(std::make_pair(fenceValue, m_CurrentCmdList));
			m_CurrentCmdList = nullptr;
			return fenceValue;
		}

	protected:
		CommandQueueDX12(Device& d, QueueType t);
		CommandQueueComPtr	m_CommandQueue;
		FenceComPtr					m_Fence;
		atomic_u64						m_FenceValue;

		void updateInFlightCmdListQueue();

		friend class DeviceDX12;
	};
}