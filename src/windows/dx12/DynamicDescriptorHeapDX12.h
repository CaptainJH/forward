//***************************************************************************************
// DynamicDescriptorHeapDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"
#include "dx12/dx12Util.h"
#include "RHI/FrameGraph/PipelineStateObjects.h"
#include <queue>
#include <functional>

namespace forward
{
	const u32 gNumDescriporPerHeap = 0x20000;

	class DeviceDX12;
	class CommandListDX12;
	class DynamicDescriptorHeapDX12
	{
	public:
		DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptorPerHeap = gNumDescriporPerHeap);

		~DynamicDescriptorHeapDX12() = default;

		void Reset();

		D3D12_CPU_DESCRIPTOR_HANDLE* PrepareDescriptorHandleCache(u32 registerCount);
		void BindGPUVisibleDescriptorHeap(CommandListDX12& commandList);
		void BindDescriptorTableToRootParam(ID3D12GraphicsCommandList* commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
		void CommitStagedDescriptors(DeviceDX12& d);
		void CommitStagedDescriptorsFrom(DeviceDX12& d, DynamicDescriptorHeapDX12& heap, u32 offset=0U);
		D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle(u32 offset = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle(u32 offset = 0);

	private:
		/**
		 * The maximum number of descriptor tables per root signature.
		 * A 32-bit mask is used to keep track of the root parameter indices that
		 * are descriptor tables.
		 */
		static const u32 MaxDescriptorTables = 1024;

		// Describes the type of descriptors that can be staged using this 
		// dynamic descriptor heap.
		// Valid values are:
		//   * D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		//   * D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
		// This parameter also determines the type of GPU visible descriptor heap to 
		// create.
		const D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorHeapType;

		// The number of descriptors to allocate in new GPU visible descriptor heaps.
		const u32 m_NumDescriptorsPerHeap;

		// The increment size of a descriptor.
		u32 m_DescriptorHandleIncrementSize;

		using DescriptorHeapPool = std::queue<DescriptorHeapComPtr>;

		DescriptorHeapPool m_DescriptorHeapPool;
		DescriptorHeapPool m_AvailableDescriptorHeaps;

		DescriptorHeapComPtr m_CurrentDescriptorHeap;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrentGPUDescriptorHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_CurrentCPUDescriptorHandle;

		/**
		 * A structure that represents a descriptor table entry in the root signature.
		 */
		struct DescriptorTableCache
		{
			DescriptorTableCache()
				: NumDescriptors(0)
				, BaseDescriptor(nullptr)
			{}

			// Reset the table cache.
			void Reset()
			{
				NumDescriptors = 0;
				BaseDescriptor = nullptr;
			}

			// The number of descriptors in this descriptor table.
			u32 NumDescriptors;
			// The pointer to the descriptor in the descriptor handle cache.
			D3D12_CPU_DESCRIPTOR_HANDLE* BaseDescriptor;
		};
		// Descriptor handle cache per descriptor table.
		std::array<DescriptorTableCache, MaxDescriptorTables> m_DescriptorTableCache;

		// The descriptor handle cache.
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_DescriptorHandleCache;

		u32 m_currentDescriptorTableOffset = 0;
		u32 m_currentDescriptorHandleCacheOffset = 0;

	private:
		// Request a descriptor heap if one is available.
		DescriptorHeapComPtr RequestDescriptorHeap(ID3D12Device* device);
		// Create a new descriptor heap if no descriptor heap is available.
		DescriptorHeapComPtr CreateDescriptorHeap(ID3D12Device* device);

		friend class DeviceDX12;
	};
}
