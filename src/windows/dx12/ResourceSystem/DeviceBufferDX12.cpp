//***************************************************************************************
// DeviceBufferDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************

#include "DeviceBufferDX12.h"
#include "RHI/ResourceSystem/Resource.h"
#include "dx12/DeviceDX12.h"

using namespace forward;

DeviceBufferDX12::DeviceBufferDX12(ID3D12GraphicsCommandList* cmdList, forward::GraphicsObject* obj, DeviceDX12& d)
	: DeviceResourceDX12(obj, d)
{
	m_cbvHandle.ptr = 0;
	m_cbvHandleGPU.ptr = 0;
	m_uavHandle.ptr = 0;

	const auto type = obj->GetType();
	Resource* res = dynamic_cast<Resource*>(obj);
	auto byteSize = res->GetNumBytes();
	auto device = m_device.GetDevice();
	if (type >= FGOT_CONSTANT_BUFFER && type <= FGOT_INDEX_BUFFER)
	{
		bool isConstantBuffer = type == FGOT_CONSTANT_BUFFER;
		ResourceUsage usage = res->GetUsage();

		if (usage == ResourceUsage::RU_IMMUTABLE && res->GetData())
		{
			CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
			assert(GetResourceState() == D3D12_RESOURCE_STATE_COMMON);
			HR(device->CreateCommittedResource(
				&heap_properties,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				GetResourceState(),
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

			SyncCPUToGPU(cmdList);
			m_gpuVirtualAddress = m_deviceResPtr->GetGPUVirtualAddress();
		}
		else if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
		{
			// Constant buffer elements need to be multiples of 256 bytes.
			// This is because the hardware can only view constant data 
			// at m*256 byte offsets and of n*256 byte lengths. 
			if (isConstantBuffer)
			{
				byteSize = CalcConstantBufferByteSize(byteSize);
			}

			CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
			HR(device->CreateCommittedResource(
				&heap_properties,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())));

			HR(m_deviceResPtr->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

			if (isConstantBuffer)
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = m_deviceResPtr->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = byteSize;
				CreateCBView(cbvDesc);
			}

			m_gpuVirtualAddress = m_deviceResPtr->GetGPUVirtualAddress();
			SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		else
		{
			assert(false && "Not Implemented yet!");
		}
	}
	else if (type == FGOT_SHADER_TABLE)
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
		HR(device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_deviceResPtr.GetAddressOf())));
		//m_resource->SetName(resourceName);

		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		HR(m_deviceResPtr->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedData)));
	}
}

DeviceBufferDX12::~DeviceBufferDX12()
{
	if (m_mappedData)
	{
		m_deviceResPtr->Unmap(0, nullptr);
	}
}

void DeviceBufferDX12::SyncCPUToGPU()
{
	auto res = m_frameGraphObjPtr.lock_down<Resource>();
	ResourceUsage usage = res->GetUsage();
	if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
	{
		memcpy(m_mappedData, res->GetData(), res->GetNumBytes());
	}
}

void DeviceBufferDX12::SyncCPUToGPU(ID3D12GraphicsCommandList* cmdList)
{
	auto res = m_frameGraphObjPtr.lock_down<Resource>();

	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = res->GetData();
	subResourceData.RowPitch = res->GetNumBytes();
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	auto transitionToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
		GetResourceState()/*D3D12_RESOURCE_STATE_COMMON*/
		, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &transitionToCopyDest);
	UpdateSubresources<1>(cmdList, m_deviceResPtr.Get(), m_stagingResPtr.Get(), 0, 0, 1, &subResourceData);
	auto transitionBack = CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResPtr.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &transitionBack);
	SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
}

D3D12_VERTEX_BUFFER_VIEW DeviceBufferDX12::VertexBufferView()
{
	auto res = m_frameGraphObjPtr.lock_down<forward::Resource>();
	assert(res->GetType() == FGOT_VERTEX_BUFFER);

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_gpuVirtualAddress;
	vbv.StrideInBytes = res->GetElementSize();
	vbv.SizeInBytes = res->GetNumBytes();

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW DeviceBufferDX12::IndexBufferView()
{
	auto res = m_frameGraphObjPtr.lock_down<forward::Resource>();
	assert(res->GetType() == FGOT_INDEX_BUFFER);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_gpuVirtualAddress;
	ibv.Format = res->GetElementSize() == sizeof(u32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = res->GetNumBytes();

	return ibv;
}

void DeviceBufferDX12::CreateCBView(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
	m_cbvHandle = m_device.AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_device.GetDevice()->CreateConstantBufferView(&desc, m_cbvHandle);
}

D3D12_CPU_DESCRIPTOR_HANDLE	DeviceBufferDX12::GetCBViewCPUHandle()
{
	assert(m_cbvHandle.ptr);
	return m_cbvHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DeviceBufferDX12::GetCBViewGPUHandle()
{
	assert(m_cbvHandleGPU.ptr);
	return m_cbvHandleGPU;
}

void DeviceBufferDX12::SetCBViewGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE inHandle)
{
	m_cbvHandleGPU = inHandle;
}

DeviceResCom12Ptr DeviceBufferDX12::AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, u64 datasize, const wchar_t* resourceName /*= nullptr*/)
{
	DeviceResCom12Ptr ret;
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	HR(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ret.GetAddressOf())));
	if (resourceName)
		ret->SetName(resourceName);
	void* pMappedData;
	ret->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	ret->Unmap(0, nullptr);
	return ret;
}

DeviceResCom12Ptr DeviceBufferDX12::AllocateUAVBuffer(ID3D12Device* pDevice, u64 bufferSize, 
	D3D12_RESOURCE_STATES initialResourceState /*= D3D12_RESOURCE_STATE_COMMON*/, const wchar_t* resourceName /*= nullptr*/)
{
	DeviceResCom12Ptr ret;
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	HR(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ret.GetAddressOf())));
	if (resourceName)
		ret->SetName(resourceName);
	return ret;
}