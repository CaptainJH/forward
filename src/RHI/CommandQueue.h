//***************************************************************************************
// CommandQueue.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "Types.h"
#include "ResourceSystem/DeviceResource.h"
#include "CommandList.h"

namespace forward
{
	enum class QueueType
	{
		Direct,
		Compute,
	};

	class CommandQueue : public intrusive_ref_counter
	{
	public:
		CommandQueue(Device& d, QueueType t, u32 maxCmdListCount) : m_device(d), m_queueType(t), m_maxCmdListCount(maxCmdListCount) {}
		virtual ~CommandQueue() {}

		// Get an available command list from the command queue.
		virtual shared_ptr<CommandList> GetCommandList() = 0;

		virtual u64 Signal() = 0;
		virtual bool IsFenceComplete(u64 fenceValue) = 0;
		virtual void Flush() = 0;
		virtual void WaitForGPU(u64 fenceValue) = 0;	// CPU wait for GPU
		// Wait for another command queue to finish.
		virtual void WaitFor(const CommandQueue& other) = 0;	// GPU wait for GPU

	protected:
		// Execute a command list.
		virtual void ExecuteCommandList() = 0;

	protected:
		Device& m_device;
		const QueueType m_queueType;
		Concurrent_Queue<shared_ptr<CommandList>> m_AvailableCmdLists;
		shared_ptr<CommandList> m_CurrentCmdList = nullptr;

		// Keep track of command allocators that are "in-flight"
		// The first member is the fence value to wait for, the second is the
		// a shared pointer to the "in-flight" command list.
		using CommandListEntry = std::tuple<u64, shared_ptr<CommandList>>;
		Concurrent_Queue<CommandListEntry> m_InFlightCmdLists;

		const u32 m_maxCmdListCount;
	};
}