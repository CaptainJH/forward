//***************************************************************************************
// DynamicDescriptorHeapDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DynamicDescriptorHeapDX12.h"
#include "dx12/RendererDX12.h"

using namespace forward;

DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	u32 incrementSize, u32 numDescriptorPerHeap/*= gNumDescriporPerHeap*/)
	: m_DescriptorHeapType(heapType)
	, m_NumDescriptorsPerHeap(numDescriptorPerHeap)
	, m_DescriptorHandleIncrementSize(incrementSize)
{
	m_DescriptorHandleCache.reserve(m_NumDescriptorsPerHeap);
}

DynamicDescriptorHeapDX12::~DynamicDescriptorHeapDX12()
{}

void DynamicDescriptorHeapDX12::Reset()
{
	m_AvailableDescriptorHeaps = m_DescriptorHeapPool;

	std::for_each(m_DescriptorTableCache.begin(), m_DescriptorTableCache.end(), [](DescriptorTableCache& cache) {
		cache.Reset();
	});
}

void DynamicDescriptorHeapDX12::StageDescriptors(u32 rootParameterIndex, u32 offset, u32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor)
{
	// Cannot stage more than the maximum number of descriptors per heap.
	// Cannot stage more than MaxDescriptorTables root parameters.
	assert(numDescriptors <= m_NumDescriptorsPerHeap && rootParameterIndex < MaxDescriptorTables);

	auto& descriptorTableCache = m_DescriptorTableCache[rootParameterIndex];

	// Check that the number of descriptors to copy does not exceed the number
	// of descriptors expected in the descriptor table.
	assert((offset + numDescriptors) <= descriptorTableCache.NumDescriptors);

	D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = descriptorTableCache.BaseDescriptor + offset;
	for (auto i = 0U; i < numDescriptors; ++i)
	{
		dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcDescriptor, i, m_DescriptorHandleIncrementSize);
	}
}

void DynamicDescriptorHeapDX12::CommitStagedDescriptors(ID3D12GraphicsCommandList* commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc)
{
	auto device = RendererContext::GetCurrentRender()->GetDevice();

	if (!m_CurrentDescriptorHeap)
	{
		m_CurrentDescriptorHeap = RequestDescriptorHeap();
		m_CurrentCPUDescriptorHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_CurrentGPUDescriptorHandle = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_CurrentDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	}

	for (auto i = 0U; i < m_DescriptorTableCache.size(); ++i)
	{
		auto numSrcDescriptors = m_DescriptorTableCache[i].NumDescriptors;
		if (numSrcDescriptors > 0)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = m_DescriptorTableCache[i].BaseDescriptor;

			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] =
			{
				m_CurrentCPUDescriptorHandle
			};
			u32 pDestDescriptorRangeSizes[] =
			{
				numSrcDescriptors
			};

			// Copy the staged CPU visible descriptors to the GPU visible descriptor heap.
			device->CopyDescriptors(1, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
				numSrcDescriptors, pSrcDescriptorHandles, nullptr, m_DescriptorHeapType);

			// Set the descriptors on the command list using the passed-in setter function.
			setFunc(commandList, i, m_CurrentGPUDescriptorHandle);

			m_CurrentCPUDescriptorHandle = m_CurrentCPUDescriptorHandle.Offset(m_DescriptorHandleIncrementSize);
			m_CurrentGPUDescriptorHandle = m_CurrentGPUDescriptorHandle.Offset(m_DescriptorHandleIncrementSize);
		}
	}
}

DescriptorHeapComPtr DynamicDescriptorHeapDX12::RequestDescriptorHeap()
{
	DescriptorHeapComPtr descriptorHeap;
	if (!m_AvailableDescriptorHeaps.empty())
	{
		descriptorHeap = m_AvailableDescriptorHeaps.front();
		m_AvailableDescriptorHeaps.pop();
	}
	else
	{
		descriptorHeap = CreateDescriptorHeap();
		m_DescriptorHeapPool.push(descriptorHeap);
	}

	return descriptorHeap;
}

DescriptorHeapComPtr DynamicDescriptorHeapDX12::CreateDescriptorHeap()
{
	auto device = RendererContext::GetCurrentRender()->GetDevice();

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = m_DescriptorHeapType;
	descriptorHeapDesc.NumDescriptors = m_NumDescriptorsPerHeap;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DescriptorHeapComPtr descriptorHeap;
	HR(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}