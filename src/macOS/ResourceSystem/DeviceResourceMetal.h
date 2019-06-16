//
//  DeviceResourceMetal.h
//  forward
//
//  Created by jhq on 2019/5/29.
//

#pragma once
#include "Types.h"
#include "render/ResourceSystem/DeviceResource.h"
#import <Metal/Metal.h>

namespace forward
{
    class DeviceResourceMetal : public DeviceResource
    {
    public:
        DeviceResourceMetal(forward::FrameGraphObject* obj);
        
        virtual ~DeviceResourceMetal();
        
        id<MTLResource>        GetDeviceResource();
        
        u32                 GetEvictionPriority() override;
        void                SetEvictionPriority(u32 EvictionPriority) override;
        
    protected:
        id<MTLResource>     m_deviceResPtr;
        
        bool                PrepareForSync();
    };
}
