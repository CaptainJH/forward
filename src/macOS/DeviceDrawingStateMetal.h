//
//  DeviceDrawingStateMetal.h
//  forward
//
//  Created by jhq on 2019/6/9.
//

#pragma once

#include "render/ResourceSystem/DeviceObject.h"
#include "render/FrameGraph/PipelineStateObjects.h"
#import <Metal/Metal.h>

namespace forward
{
    class DeviceDepthStencilStateMetal : public DeviceObject
    {
    public:
        DeviceDepthStencilStateMetal(id<MTLDevice> device, forward::DepthStencilState* ds);
        
        void Bind(id<MTLRenderCommandEncoder> encoder);
        
    private:
        id<MTLDepthStencilState> m_deviceState;
    };
}
