//***************************************************************************************
// DynamicDescriptorHeapDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"
#include "dx12/dx12Util.h"
#include <list>

namespace forward
{
	const u32 gNumDescriporPerHeap = 32;

	class DynamicDescriptorHeapDX12
	{
	public:
		DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			u32 numDescriptorPerHeap = gNumDescriporPerHeap);

		~DynamicDescriptorHeapDX12();

		void Reset();

	private:
		// Describes the type of descriptors that can be staged using this 
		// dynamic descriptor heap.
		// Valid values are:
		//   * D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		//   * D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
		// This parameter also determines the type of GPU visible descriptor heap to 
		// create.
		D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorHeapType;

		// The number of descriptors to allocate in new GPU visible descriptor heaps.
		u32 m_NumDescriptorsPerHeap;

		// The increment size of a descriptor.
		const u32 m_DescriptorHandleIncrementSize;

		using DescriptorHeapPool = std::list<DescriptorHeapComPtr>;

		DescriptorHeapPool m_DescriptorHeapPool;
		DescriptorHeapPool m_AvailableDescriptorHeaps;

		DescriptorHeapComPtr m_CurrentDescriptorHeap;
	};
}
