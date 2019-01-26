//***************************************************************************************
// DynamicDescriptorHeapDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DynamicDescriptorHeapDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	u32 numDescriptorPerHeap/*= gNumDescriporPerHeap*/)
	: m_DescriptorHeapType(heapType)
	, m_NumDescriptorsPerHeap(numDescriptorPerHeap)
	, m_DescriptorHandleIncrementSize(RendererContext::GetCurrentRender()->GetDescriptorHandleIncrementSize(heapType))
{
}

DynamicDescriptorHeapDX12::~DynamicDescriptorHeapDX12()
{}

void DynamicDescriptorHeapDX12::Reset()
{

}

