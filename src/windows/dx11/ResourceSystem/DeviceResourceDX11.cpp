//***************************************************************************************
// DeviceResourceDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceResourceDX11.h"
#include "render/ResourceSystem/FrameGraphResource.h"

using namespace forward;

DeviceResourceDX11::DeviceResourceDX11(forward::FrameGraphObject* obj)
	: DeviceResource(obj)
{
}

DeviceResourceDX11::~DeviceResourceDX11()
{

}

DeviceResComPtr DeviceResourceDX11::GetDeviceResource()
{
	return m_deviceResPtr;
}

u32 DeviceResourceDX11::GetEvictionPriority()
{
	u32 priority = 0;

	if (m_deviceResPtr)
	{
		priority = m_deviceResPtr->GetEvictionPriority();
	}

	return priority;
}

void DeviceResourceDX11::SetEvictionPriority(u32 EvictionPriority)
{
	if (m_deviceResPtr)
	{
		m_deviceResPtr->SetEvictionPriority(EvictionPriority);
	}
}

bool DeviceResourceDX11::PrepareForSync()
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