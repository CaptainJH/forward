//***************************************************************************************
// DynamicDescriptorHeapDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"
#include "dx12/d3dx12.h"

namespace forward
{
	const u32 gNumDescriporPerHeap = 32;

	class DynamicDescriptorHeap
	{
	public:
		DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			u32 numDescriptorPerHeap = gNumDescriporPerHeap);

		~DynamicDescriptorHeap();
	};
}
