//
//  DevicePipelineStateObjectMetal.m
//  HelloMac
//
//  Created by jhq on 2019/6/9.
//

#include "DevicePipelineStateObjectMetal.h"
#include "FrameGraph/PipelineStateObjects.h"
#include "RendererMetal.h"
#include "ShaderMetal.h"
#include "ResourceSystem/DeviceTextureMetal.h"
#include "macUtil.h"

using namespace forward;

DevicePipelineStateObjectMetal::DevicePipelineStateObjectMetal(RendererMetal* render, const PipelineStateObject& pso)
: DeviceObject(nullptr)
, m_pso(pso)
{
    auto device = render->GetDevice();
    m_renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    
    bool bUseDefaultRenderPassSetting = true;
    // setup m_renderPassDesc
    for (auto i = 0U; i < pso.m_OMState.m_renderTargetResources.size(); ++i)
    {
        auto rt = pso.m_OMState.m_renderTargetResources[i];
        if (rt)
        {
            if(rt->Name() == "DefaultRT")
            {
                m_renderPassDesc.colorAttachments[i].texture = render->GetDefaultDeviceRT();
            }
            else
            {
                assert(rt->DeviceObject());
                auto deviceTexPtr = device_cast<DeviceTextureMetal*>(rt);
                m_renderPassDesc.colorAttachments[i].texture = deviceTexPtr->GetMetalTexturePtr();
            }
            
            m_renderPassDesc.colorAttachments[i].storeAction = MTLStoreActionStore;
            m_renderPassDesc.colorAttachments[i].loadAction = MTLLoadActionClear;
            bUseDefaultRenderPassSetting = false;
        }
    }
    
    auto dsFormat = render->GetCurrentDSBufferPixelFormat();
    auto ds = pso.m_OMState.m_depthStencilResource;
    if(ds)
    {
        if(ds->Name() == "DefaultDS")
        {
            m_renderPassDesc.depthAttachment.texture = render->GetDefaultDeviceDS();
            m_renderPassDesc.stencilAttachment.texture = render->GetDefaultDeviceDS();
        }
        else
        {
            assert(ds->DeviceObject());
            auto deviceTexPtr = device_cast<DeviceTextureMetal*>(ds);
            m_renderPassDesc.depthAttachment.texture = deviceTexPtr->GetMetalTexturePtr();
            m_renderPassDesc.stencilAttachment.texture = deviceTexPtr->GetMetalTexturePtr();
            dsFormat = deviceTexPtr->GetMetalTexturePtr().pixelFormat;
        }
        
        m_renderPassDesc.depthAttachment.storeAction = MTLStoreActionStore;
        m_renderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
    }
    else
    {
        dsFormat = MTLPixelFormatInvalid;
    }
    
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    
    // setup pipelineDesc
    auto vs = forward::device_cast<ShaderMetal*>(pso.m_VSState.m_shader);
    auto ps = device_cast<ShaderMetal*>(pso.m_PSState.m_shader);
    pipelineDesc.vertexFunction = vs->GetEntry();
    pipelineDesc.fragmentFunction = ps->GetEntry();
    pipelineDesc.colorAttachments[0].pixelFormat = bUseDefaultRenderPassSetting ?
            render->GetCurrentBackBufferPixelFormat() : m_renderPassDesc.colorAttachments[0].texture.pixelFormat;
    pipelineDesc.depthAttachmentPixelFormat = dsFormat;
    pipelineDesc.stencilAttachmentPixelFormat = dsFormat;
    
    // setup vertex layout
    FrameGraphVertexBuffer* vbuffer = pso.m_IAState.m_vertexBuffers[0].get();
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    if(vbuffer && m_pso.m_VSState.m_shader)
    {
        const auto& vertexFormat = vbuffer->GetVertexFormat();
        auto numElements = vertexFormat.GetNumAttributes();
        for (auto i = 0U; i < numElements; ++i)
        {
            VASemantic semantic;
            DataFormatType type;
            u32 unit, offset;
            vertexFormat.GetAttribute(i, semantic, type, unit, offset);
            
            vertexDesc.attributes[i].format = Convert2MetalVertexFormat(type);
            vertexDesc.attributes[i].bufferIndex = 0;
            vertexDesc.attributes[i].offset = offset;
        }
        
        vertexDesc.layouts[0].stride = vertexFormat.GetVertexSize();
        vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    }
    pipelineDesc.vertexDescriptor = vertexDesc;
    
    NSError *error = NULL;
    m_devicePipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    [pipelineDesc release];
}

void DevicePipelineStateObjectMetal::Bind(id<MTLRenderCommandEncoder> renderEncoder)
{
    [renderEncoder setRenderPipelineState:m_devicePipelineState];
}

MTLPrimitiveType DevicePipelineStateObjectMetal::GetPSOPrimitiveType() const
{
    const auto topology = m_pso.m_IAState.m_topologyType;
    assert(topology > PT_UNDEFINED && topology < PT_LINELIST_ADJ);
    
    static MTLPrimitiveType msTopology[] =
    {
        MTLPrimitiveTypePoint,
        MTLPrimitiveTypeLine,
        MTLPrimitiveTypeLineStrip,
        MTLPrimitiveTypeTriangle,
        MTLPrimitiveTypeTriangleStrip,
    };
    
    return msTopology[topology - 1];
}

MTLRenderPassDescriptor* DevicePipelineStateObjectMetal::GetRenderPassDesc()
{
    return m_renderPassDesc;
}
