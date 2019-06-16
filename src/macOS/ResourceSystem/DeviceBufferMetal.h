//
//  DeviceBufferMetal.h
//  forward
//
//  Created by jhq on 2019/5/31.
//

#pragma once

#include "DeviceResourceMetal.h"

namespace forward
{
    class FrameGraphBuffer;
    
    class DeviceBufferMetal : public DeviceResourceMetal
    {
    public:
        DeviceBufferMetal(id<MTLDevice> device, forward::FrameGraphBuffer* obj);
        
        id<MTLBuffer> GetMetalBufferPtr();
        
        void SyncCPUToGPU() override;
        
    };
}
