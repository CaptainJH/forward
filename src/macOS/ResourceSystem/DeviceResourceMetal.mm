//
//  DeviceResourceMetal.m
//  HelloMac
//
//  Created by jhq on 2019/5/29.
//

#include "DeviceResourceMetal.h"
#include "render/ResourceSystem/FrameGraphResource.h"

using namespace forward;

DeviceResourceMetal::DeviceResourceMetal(forward::FrameGraphObject* obj)
: DeviceResource(obj)
{
    m_deviceResPtr = nil;
}

DeviceResourceMetal::~DeviceResourceMetal()
{
    
}

id<MTLResource> DeviceResourceMetal::GetDeviceResource()
{
    return m_deviceResPtr;
}

u32 DeviceResourceMetal::GetEvictionPriority()
{
    return 0;
}


void DeviceResourceMetal::SetEvictionPriority(u32 EvictionPriority)
{
}
