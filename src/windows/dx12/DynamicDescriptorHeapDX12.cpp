//***************************************************************************************
// DynamicDescriptorHeapDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DynamicDescriptorHeapDX12.h"
#include "dx12/DeviceDX12.h"
#include "dx12/CommandListDX12.h"
#include "dx12/ShaderSystem/ShaderDX12.h"

using namespace forward;

DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptorPerHeap/*= gNumDescriporPerHeap*/)
	: m_DescriptorHeapType(heapType)
	, m_NumDescriptorsPerHeap(numDescriptorPerHeap)
	, m_DescriptorHandleIncrementSize(0)
{
	m_DescriptorHandleCache.resize(m_NumDescriptorsPerHeap);
	for (auto& cache : m_DescriptorHandleCache)
	{
		cache.ptr = 0;
	}
}

void DynamicDescriptorHeapDX12::Reset()
{
	m_AvailableDescriptorHeaps = m_DescriptorHeapPool;

	std::for_each(m_DescriptorTableCache.begin(), m_DescriptorTableCache.end(), [](DescriptorTableCache& cache) {
		cache.Reset();
	});

	m_CurrentDescriptorHeap.Reset();
	m_CurrentCPUDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
	m_CurrentGPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);

	m_currentDescriptorTableOffset = 0;
	m_currentDescriptorHandleCacheOffset = 0;
}

void DynamicDescriptorHeapDX12::CommitStagedDescriptors(DeviceDX12& d)
{
	if (m_currentDescriptorTableOffset == 0) return;

	if (m_CurrentCPUDescriptorHandle == CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT))
		m_CurrentCPUDescriptorHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	u32 totalDescriptorCount = 0;
	for (auto i = 0U; i < m_currentDescriptorTableOffset; ++i)
		totalDescriptorCount += m_DescriptorTableCache[i].NumDescriptors;
	
	if (totalDescriptorCount > 0)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = m_DescriptorTableCache.begin()->BaseDescriptor;

		D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] =
		{
			m_CurrentCPUDescriptorHandle
		};
		u32 pDestDescriptorRangeSizes[] =
		{
			totalDescriptorCount
		};

		// Copy the staged CPU visible descriptors to the GPU visible descriptor heap.
		d.GetDevice()->CopyDescriptors(1, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
			totalDescriptorCount, pSrcDescriptorHandles, nullptr, m_DescriptorHeapType);

		m_CurrentCPUDescriptorHandle =
			m_CurrentCPUDescriptorHandle.Offset(totalDescriptorCount, m_DescriptorHandleIncrementSize);
	}
}

void DynamicDescriptorHeapDX12::CommitStagedDescriptorsFrom(DeviceDX12& d, DynamicDescriptorHeapDX12& srcHeap, u32 offset)
{
	if (srcHeap.m_currentDescriptorTableOffset == 0) return;

	CD3DX12_CPU_DESCRIPTOR_HANDLE dstHandle;
	dstHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	dstHandle = dstHandle.Offset(offset, m_DescriptorHandleIncrementSize);

	u32 totalDescriptorCount = 0;
	for (auto i = 0U; i < srcHeap.m_currentDescriptorTableOffset; ++i)
		totalDescriptorCount += srcHeap.m_DescriptorTableCache[i].NumDescriptors;

	if (totalDescriptorCount > 0)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = srcHeap.m_DescriptorTableCache.begin()->BaseDescriptor;

		for (auto idx = 0U; idx < srcHeap.m_DescriptorHandleCache.size();)
		{
			auto handle = srcHeap.m_DescriptorHandleCache[idx];

			if (!handle.ptr)
			{
				++idx;
				pSrcDescriptorHandles += 1;
				dstHandle.Offset(1, m_DescriptorHandleIncrementSize);
				continue;
			}

			auto startIdx = idx;
			while (handle.ptr)
			{
				if (++idx >= srcHeap.m_DescriptorHandleCache.size())
					break;
				handle = srcHeap.m_DescriptorHandleCache[idx];
			}
			const auto rangeCount = idx - startIdx;

			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] =
			{
				dstHandle
			};
			u32 pDestDescriptorRangeSizes[] =
			{
				rangeCount
			};

			// Copy the staged CPU visible descriptors to the GPU visible descriptor heap.
			d.GetDevice()->CopyDescriptors(1, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
				rangeCount, pSrcDescriptorHandles, nullptr, m_DescriptorHeapType);

			dstHandle = dstHandle.Offset(rangeCount, m_DescriptorHandleIncrementSize);
			pSrcDescriptorHandles += rangeCount;
		}

	}
}

void DynamicDescriptorHeapDX12::BindDescriptorTableToRootParam(ID3D12GraphicsCommandList* commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc)
{
	if (m_currentDescriptorTableOffset == 0) return;

	if (m_CurrentGPUDescriptorHandle == CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT))
		m_CurrentGPUDescriptorHandle = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	auto& descriptorTableCache = m_DescriptorTableCache[m_currentDescriptorTableOffset - 1];
	if (descriptorTableCache.NumDescriptors > 0)
	{
		// Set the descriptors on the command list using the passed-in setter function.
		setFunc(commandList, 0, m_CurrentGPUDescriptorHandle);
		m_CurrentGPUDescriptorHandle =
			m_CurrentGPUDescriptorHandle.Offset(descriptorTableCache.NumDescriptors, m_DescriptorHandleIncrementSize);
	}
}

void DynamicDescriptorHeapDX12::BindGPUVisibleDescriptorHeap(CommandListDX12& commandList)
{
	if (!m_CurrentDescriptorHeap)
		m_CurrentDescriptorHeap = RequestDescriptorHeap(commandList.GetDeviceDX12().GetDevice());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CurrentDescriptorHeap.Get() };
	commandList.GetDeviceCmdListPtr()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

DescriptorHeapComPtr DynamicDescriptorHeapDX12::RequestDescriptorHeap(ID3D12Device* device)
{
	DescriptorHeapComPtr descriptorHeap;
	if (!m_AvailableDescriptorHeaps.empty())
	{
		descriptorHeap = m_AvailableDescriptorHeaps.front();
		m_AvailableDescriptorHeaps.pop();
	}
	else
	{
		descriptorHeap = CreateDescriptorHeap(device);
		m_DescriptorHeapPool.push(descriptorHeap);
	}

	return descriptorHeap;
}

DescriptorHeapComPtr DynamicDescriptorHeapDX12::CreateDescriptorHeap(ID3D12Device* device)
{
	if (m_DescriptorHandleIncrementSize == 0)
	{
		m_DescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(m_DescriptorHeapType);
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = m_DescriptorHeapType;
	descriptorHeapDesc.NumDescriptors = m_NumDescriptorsPerHeap;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DescriptorHeapComPtr descriptorHeap;
	HR(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE* DynamicDescriptorHeapDX12::PrepareDescriptorHandleCache(u32 registerCount)
{
	if (registerCount > 0)
	{
		DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[m_currentDescriptorTableOffset++];
		descriptorTableCache.BaseDescriptor = (&*m_DescriptorHandleCache.begin()) + m_currentDescriptorHandleCacheOffset;
		descriptorTableCache.NumDescriptors = registerCount;
		m_currentDescriptorHandleCacheOffset += registerCount;

		return descriptorTableCache.BaseDescriptor;
	}
	return nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeapDX12::GPUHandle(u32 offset)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;
	gpuHeapStart = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	return gpuHeapStart.Offset(offset, m_DescriptorHandleIncrementSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DynamicDescriptorHeapDX12::CPUHandle(u32 offset)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
	cpuHeapStart = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	return cpuHeapStart.Offset(offset, m_DescriptorHandleIncrementSize);
}