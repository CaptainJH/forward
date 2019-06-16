//
//  DeviceBufferMetal.m
//  HelloMac
//
//  Created by jhq on 2019/5/31.
//

#include "DeviceBufferMetal.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"

using namespace forward;

DeviceBufferMetal::DeviceBufferMetal(id<MTLDevice> device, forward::FrameGraphBuffer* obj)
: DeviceResourceMetal(obj)
{
    auto type = obj->GetType();
    bool isConstantBuffer = type == FGOT_CONSTANT_BUFFER;
    if (type == FGOT_VERTEX_BUFFER)
    {
        
    }
    else if(type == FGOT_INDEX_BUFFER)
    {
        
    }
    
    FrameGraphResource* res = obj;
    auto byteSize = res->GetNumBytes();
    ResourceUsage usage = res->GetUsage();
    
    if (usage == ResourceUsage::RU_IMMUTABLE && res->GetData())
    {
        auto bufferPtr = [device newBufferWithLength:byteSize options:MTLResourceStorageModeShared];
        m_deviceResPtr = bufferPtr;
        
        memcpy(bufferPtr.contents, res->GetData(), byteSize);
    }
    else if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
    {
        auto bufferPtr = [device newBufferWithLength:byteSize options:MTLResourceStorageModeShared];
        m_deviceResPtr = bufferPtr;
        
        memcpy(bufferPtr.contents, res->GetData(), byteSize);
    }
    else
    {
        assert(false && "Not Implemented yet!");
    }
}

id<MTLBuffer> DeviceBufferMetal::GetMetalBufferPtr()
{
    id<MTLBuffer> ret = (id<MTLBuffer>)m_deviceResPtr;
    return ret;
}


void DeviceBufferMetal::SyncCPUToGPU()
{
    auto res = m_frameGraphObjPtr.lock_down<FrameGraphResource>();
    ResourceUsage usage = res->GetUsage();
    if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
    {
        auto devicePtr = GetMetalBufferPtr();
        void* ptr = [devicePtr contents];
        memcpy(ptr, res->GetData(), res->GetNumBytes());
    }
}
