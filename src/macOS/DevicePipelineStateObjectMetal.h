//
//  DevicePipelineStateObjectMetal.h
//  forward
//
//  Created by jhq on 2019/6/9.
//

#pragma once
#include "render/ResourceSystem/DeviceObject.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"
#include "render/ShaderSystem/FrameGraphShader.h"
#import <Metal/Metal.h>

namespace forward
{
    struct PipelineStateObject;
    class RendererMetal;
    
    
    class DevicePipelineStateObjectMetal : public DeviceObject
    {
    public:
        DevicePipelineStateObjectMetal(RendererMetal* render, const PipelineStateObject& pso);
        
        void Bind(id<MTLRenderCommandEncoder> renderEncoder);
        
        MTLPrimitiveType GetPSOPrimitiveType() const;
        MTLRenderPassDescriptor* GetRenderPassDesc();
        
    private:
        const PipelineStateObject&  m_pso;
        
        id<MTLRenderPipelineState>  m_devicePipelineState;
        MTLRenderPassDescriptor*    m_renderPassDesc = nullptr;
    };
}
