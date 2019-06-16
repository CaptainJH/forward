//
//  DeviceTextureMetal.h
//  forward
//
//  Created by jhq on 2019/6/11.
//

#pragma once
#include "DeviceResourceMetal.h"

namespace forward
{
    class FrameGraphTexture;
    class DeviceTextureMetal : public DeviceResourceMetal
    {
    public:
        DeviceTextureMetal(id<MTLDevice> device, FrameGraphTexture* tex);
        
        static shared_ptr<FrameGraphTexture2D> BuildDeviceTexture2DDX11(const std::string& name, id<MTLTexture> tex, ResourceUsage usage=RU_IMMUTABLE);
        
        id<MTLTexture> GetMetalTexturePtr();
        void SyncCPUToGPU() override;
        void SyncGPUToCPU();
    };
}
