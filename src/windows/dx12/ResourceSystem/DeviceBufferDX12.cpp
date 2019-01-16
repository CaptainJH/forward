//***************************************************************************************
// DeviceBufferDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************

#include "DeviceBufferDX12.h"
#include "render/ResourceSystem/FrameGraphResource.h"

using namespace forward;

DeviceBufferDX12::DeviceBufferDX12(ID3D12Device* device, forward::FrameGraphObject* obj)
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

}

void DeviceBufferDX12::SyncCPUToGPU()
{

}