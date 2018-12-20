//***************************************************************************************
// DeviceResourceDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceResourceDX12.h"
#include "render/ResourceSystem/FrameGraphResource.h"

using namespace forward;

DeviceResourceDX12::DeviceResourceDX12(forward::FrameGraphObject* obj)
	: DeviceResource(obj)
	, m_currentUsageState(D3D12_RESOURCE_STATE_COMMON)
	, m_transitioningState((D3D12_RESOURCE_STATES)-1)
{
}

DeviceResourceDX12::~DeviceResourceDX12()
{

}

DeviceResCom12Ptr DeviceResourceDX12::GetDeviceResource()
{
	return m_deviceResPtr;
}

u32 DeviceResourceDX12::GetEvictionPriority()
{
	u32 priority = 0;

	if (m_deviceResPtr)
	{
		/// TODO: implement for DX12
		///priority = m_deviceResPtr->GetEvictionPriority();
	}

	return priority;
}

void DeviceResourceDX12::SetEvictionPriority(u32 /*EvictionPriority*/)
{
	if (m_deviceResPtr)
	{
		/// TODO: implement for DX12
		/// m_deviceResPtr->SetEvictionPriority(EvictionPriority);
	}
}

bool DeviceResourceDX12::PrepareForSync()
{
	assert(m_deviceResPtr);
	assert(m_stagingResPtr);
	assert(!m_frameGraphObjPtr.expired());

	if (m_frameGraphObjPtr.lock_down<FrameGraphResource>()->GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
	{
		return true;
	}
	else
	{
		Log::Get().Write(L"Current resource usage setting doesn't support staging!");
		return false;
	}
}