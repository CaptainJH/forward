//***************************************************************************************
// DeviceBufferDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************

#include "DeviceBufferDX12.h"
#include "render/ResourceSystem/FrameGraphResource.h"

using namespace forward;

DeviceBufferDX12::DeviceBufferDX12(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, forward::FrameGraphObject* obj)
	: DeviceResourceDX12(obj)
{
	m_srvHandle.ptr = 0;
	m_uavHandle.ptr = 0;

	auto type = obj->GetType();
	if (type == FGOT_VERTEX_BUFFER)
	{

	}
	else if(type == FGOT_INDEX_BUFFER)
	{

	}

	FrameGraphResource* res = dynamic_cast<FrameGraphResource*>(obj);
	auto byteSize = res->GetNumBytes();

	CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	HR(device->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	CD3DX12_HEAP_PROPERTIES heap_staging_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	HR(device->CreateCommittedResource(
		&heap_staging_properties,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_stagingResPtr.GetAddressOf())));

	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = res->GetData();
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	auto transitionToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &transitionToCopyDest);
	UpdateSubresources<1>(cmdList, m_deviceResPtr.Get(), m_stagingResPtr.Get(), 0, 0, 1, &subResourceData);
	auto transitionBack = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &transitionBack);

}

void DeviceBufferDX12::SyncCPUToGPU()
{

}

D3D12_VERTEX_BUFFER_VIEW DeviceBufferDX12::VertexBufferView()
{
	auto res = m_frameGraphObjPtr.lock_down<forward::FrameGraphResource>();
	assert(res->GetType() == FGOT_VERTEX_BUFFER);

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_deviceResPtr->GetGPUVirtualAddress();
	vbv.StrideInBytes = res->GetElementSize();
	vbv.SizeInBytes = res->GetNumBytes();

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW DeviceBufferDX12::IndexBufferView()
{
	auto res = m_frameGraphObjPtr.lock_down<forward::FrameGraphResource>();
	assert(res->GetType() == FGOT_INDEX_BUFFER);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_deviceResPtr->GetGPUVirtualAddress();
	ibv.Format = res->GetElementSize() == sizeof(u32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = res->GetNumBytes();

	return ibv;
}