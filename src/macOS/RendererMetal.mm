//
//  RendererMetal.m
//  forwardMetal
//
//  Created by jhq on 2019/5/22.
//

#include "RendererMetal.h"
#include "utilities/Utils.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/FrameGraphObject.h"
#include "ShaderMetal.h"
#include "DevicePipelineStateObjectMetal.h"
#include "DeviceDrawingStateMetal.h"
#include "ResourceSystem/DeviceBufferMetal.h"
#include "ResourceSystem/DeviceTextureMetal.h"
#include "utilities/FileSaver.h"

using namespace forward;

RendererMetal::~RendererMetal()
{
    
}

RendererAPI RendererMetal::GetRendererAPI() const
{
    return RendererAPI::Metal;
}

void RendererMetal::DrawRenderPass(RenderPass& pass)
{
    auto& pso = pass.GetPSO();
    
    if(!pso.m_VSState.m_shader->DeviceObject())
    {
        auto deviceVS = make_shared<ShaderMetal>(m_device, pso.m_VSState.m_shader.get());
        pso.m_VSState.m_shader->SetDeviceObject(deviceVS);
    }

    if(!pso.m_PSState.m_shader->DeviceObject())
    {
        auto devicePS = make_shared<ShaderMetal>(m_device, pso.m_PSState.m_shader.get());
        pso.m_PSState.m_shader->SetDeviceObject(devicePS);
    }
    
    // create & update device vertex buffer
    for (auto i = 0U; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
    {
        auto vb = pso.m_IAState.m_vertexBuffers[i];
        if (vb && !vb->DeviceObject())
        {
            auto deviceVB = make_shared<DeviceBufferMetal>(GetDevice(), vb.get());
            vb->SetDeviceObject(deviceVB);
        }
    }
    
    // create & update device index buffer
    auto ib = pso.m_IAState.m_indexBuffer;
    if (ib && !ib->DeviceObject())
    {
        auto deviceIB = make_shared<DeviceBufferMetal>(GetDevice(), ib.get());
        ib->SetDeviceObject(deviceIB);
    }
    
    if(!pso.m_OMState.m_dsState.DeviceObject())
    {
        auto deviceDS = make_shared<DeviceDepthStencilStateMetal>(GetDevice(), &pso.m_OMState.m_dsState);
        pso.m_OMState.m_dsState.SetDeviceObject(deviceDS.get());
    }
    
    // setup VS
    for (auto cb : pso.m_VSState.m_constantBuffers)
    {
        if(cb)
        {
            if (!cb->DeviceObject())
            {
                auto deviceCB = forward::make_shared<DeviceBufferMetal>(GetDevice(), cb.get());
                cb->SetDeviceObject(deviceCB);
            }
            
            auto deviceCB = device_cast<DeviceBufferMetal*>(cb);
            deviceCB->SyncCPUToGPU();
        }
    }
    
    // setup PS
    for(auto cb : pso.m_PSState.m_constantBuffers)
    {
        if(cb)
        {
            if (!cb->DeviceObject())
            {
                auto deviceCB = forward::make_shared<DeviceBufferMetal>(GetDevice(), cb.get());
                cb->SetDeviceObject(deviceCB);
            }
            
            auto deviceCB = device_cast<DeviceBufferMetal*>(cb);
            deviceCB->SyncCPUToGPU();
        }
    }
    
    for (auto i = 0U; i < pso.m_PSState.m_shaderResources.size(); ++i)
    {
        auto res = pso.m_PSState.m_shaderResources[i];
        if (res)
        {
            if (!res->DeviceObject())
            {
                if (dynamic_cast<FrameGraphTexture2D*>(res.get()))
                {
                    auto deviceTex = forward::make_shared<DeviceTextureMetal>(GetDevice(), dynamic_cast<FrameGraphTexture2D*>(res.get()));
                    res->SetDeviceObject(deviceTex);
                }
//                else if (dynamic_cast<FrameGraphTextureCube*>(res.get()))
//                {
//                    auto deviceTex = forward::make_shared<DeviceTextureCubeDX11>(m_pDevice.Get(), dynamic_cast<FrameGraphTextureCube*>(res.get()));
//                    res->SetDeviceObject(deviceTex);
//                }
            }
        }
    }
    
    bool bUseDefaultRenderPassSetting = true;
    for (auto i = 0U; i < pso.m_OMState.m_renderTargetResources.size(); ++i)
    {
        auto rt = pso.m_OMState.m_renderTargetResources[i];
        if (rt && rt->Name() != "DefaultRT")
        {
            if (!rt->DeviceObject())
            {
                auto deviceRT = make_shared<DeviceTextureMetal>(GetDevice(), rt.get());
                rt->SetDeviceObject(deviceRT);
            }
            
            bUseDefaultRenderPassSetting = false;
        }
    }
    
    if(pso.m_OMState.m_depthStencilResource)
    {
        m_view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    }
    else
    {
        m_view.depthStencilPixelFormat = MTLPixelFormatInvalid;
    }
    
    if(!pso.m_devicePSO)
    {
        pso.m_devicePSO = make_shared<DevicePipelineStateObjectMetal>(this, pso);
    }
    
    auto devicePSO = dynamic_cast<DevicePipelineStateObjectMetal*>(pso.m_devicePSO.get());

    // Obtain a renderPassDescriptor generated from the view's drawable textures
    MTLRenderPassDescriptor *renderPassDescriptor = bUseDefaultRenderPassSetting ?
        m_view.currentRenderPassDescriptor : devicePSO->GetRenderPassDesc();
    
    auto actionFlag = pass.GetRenderPassFlags();
    for(auto i = 0U; i < pso.m_OMState.m_renderTargetResources.size(); ++i)
    {
        auto rt = pso.m_OMState.m_renderTargetResources[i];
        if(!rt)
            continue;
        
        if(actionFlag & RenderPass::OF_CLEAN_RT)
        {
            renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
        }
        else
        {
            renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
        }
        renderPassDescriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
    }
    
    if(pso.m_OMState.m_depthStencilResource)
    {
        if(actionFlag & RenderPass::OF_CLEAN_DS)
        {
            renderPassDescriptor.depthAttachment.clearDepth = 1.0f;
            renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
            renderPassDescriptor.stencilAttachment.clearStencil = 0;
            renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
        }
        else
        {
            renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
            renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
        }

        renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionStore;
    }

    if(renderPassDescriptor != nil)
    {
        // Create a render command encoder so we can render into something
        id<MTLRenderCommandEncoder> renderEncoder =
        [m_currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        m_currentEncoder = renderEncoder;

        
        i32 vsIndex = 0;
        for (auto vb : pso.m_IAState.m_vertexBuffers)
        {
            if(vb)
            {
                auto deviceVB = device_cast<DeviceBufferMetal*>(vb);
                [renderEncoder setVertexBuffer:deviceVB->GetMetalBufferPtr() offset:0 atIndex:vsIndex++];
            }
        }
        for (auto cb : pso.m_VSState.m_constantBuffers)
        {
            if (cb)
            {
                auto deviceCB = device_cast<DeviceBufferMetal*>(cb);
                [renderEncoder setVertexBuffer:deviceCB->GetMetalBufferPtr() offset:0 atIndex:vsIndex++];
            }
        }
        
        i32 psBufferIndex = 0;
        for(auto cb : pso.m_PSState.m_constantBuffers)
        {
            if(cb)
            {
                auto deviceCB = device_cast<DeviceBufferMetal*>(cb);
                [renderEncoder setFragmentBuffer:deviceCB->GetMetalBufferPtr() offset:0 atIndex:psBufferIndex++];
            }
        }
        i32 psTexIndex = 0;
        for(auto res : pso.m_PSState.m_shaderResources)
        {
            if(res)
            {
                auto deviceRes = device_cast<DeviceTextureMetal*>(res);
                [renderEncoder setFragmentTexture:deviceRes->GetMetalTexturePtr() atIndex:psTexIndex++];
            }
        }
        
        if(ib)
        {
            m_currentIndexBuffer = device_cast<DeviceBufferMetal*>(ib)->GetMetalBufferPtr();
        }
        
        if(pso.m_OMState.m_depthStencilResource)
        {
            auto deviceDS = device_cast<DeviceDepthStencilStateMetal*>(&pso.m_OMState.m_dsState);
            deviceDS->Bind(renderEncoder);
        }

        auto devicePSO = dynamic_cast<DevicePipelineStateObjectMetal*>(pso.m_devicePSO.get());
        devicePSO->Bind(renderEncoder);
        m_currentPrimitiveType = devicePSO->GetPSOPrimitiveType();

        
        // Draw
        pass.Execute(*this);
        
        [renderEncoder endEncoding];
    }
}

void RendererMetal::DeleteResource(ResourcePtr ptr)
{
    
}

void RendererMetal::OnResize(u32 width, u32 height)
{
    
}

bool RendererMetal::Initialize(SwapChainConfig& config, bool bOffScreen)
{
    return false;
}

void RendererMetal::Shutdown()
{
    
}

void RendererMetal::Draw(u32 vertexNum, u32 startVertexLocation/*=0*/)
{
    [m_currentEncoder drawPrimitives:m_currentPrimitiveType vertexStart:startVertexLocation vertexCount:vertexNum];
}

void RendererMetal::DrawIndexed(u32 indexCount)
{
    [m_currentEncoder drawIndexedPrimitives:m_currentPrimitiveType indexCount:indexCount indexType:MTLIndexTypeUInt32 indexBuffer:m_currentIndexBuffer indexBufferOffset:0];
}

void RendererMetal::ResolveResource(FrameGraphTexture2D* dst, FrameGraphTexture2D* src)
{
    
}

void RendererMetal::SaveRenderTarget(const std::wstring& filename, PipelineStateObject& pso)
{
    auto commandBuffer = [m_commandQueue commandBuffer];
    auto rtPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("OffScreenRT");
    auto deviceRT = device_cast<DeviceTextureMetal*>(rtPtr);
    auto blitEncoder = [commandBuffer blitCommandEncoder];
    [blitEncoder synchronizeResource:deviceRT->GetMetalTexturePtr()];
    [blitEncoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    deviceRT->SyncGPUToCPU();
    
    u8* tempBuffer = new u8[rtPtr->GetNumBytes()];
    memcpy(tempBuffer, rtPtr->GetData(), rtPtr->GetNumBytes());
    if (rtPtr->GetElementSize() >= 3 && rtPtr->GetFormat() == DF_R8G8B8A8_UNORM)
    {
        // transform from RGBA to BGRA
        for (auto i = 0U; i < rtPtr->GetNumBytes(); i += rtPtr->GetElementSize())
        {
            std::swap(tempBuffer[i], tempBuffer[i + 2]);
        }
    }
    
    FileSaver outfile;
    outfile.SaveAsBMP(filename, tempBuffer, rtPtr->GetWidth(), rtPtr->GetHeight());
    SAFE_DELETE_ARRAY(tempBuffer);
}

void RendererMetal::DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color)
{
    m_textFont->Typeset(m_width, m_height, x, y, color, msg);
    if (m_currentFrameGraph)
    {
        m_currentFrameGraph->DrawRenderPass(m_textRenderPass);
    }
}

void RendererMetal::EndDrawFrameGraph()
{
    m_currentFrameGraph->LinkInfo();
    //CompileCurrentFrameGraph();
    
    m_currentCommandBuffer = [m_commandQueue commandBuffer];
    auto renderPassDB = m_currentFrameGraph->GetRenderPassDB();
    for (auto renderPass : renderPassDB)
    {
        m_currentPass = renderPass.m_renderPass;
        DrawRenderPass(*m_currentPass);
    }
    
    // Schedule a present once the framebuffer is complete using the current drawable
    [m_currentCommandBuffer presentDrawable:m_view.currentDrawable];
    
    // Finalize rendering here & push the command buffer to the GPU
    [m_currentCommandBuffer commit];
    
    [m_currentCommandBuffer waitUntilCompleted];
    
    m_currentFrameGraph = nullptr;
}

shared_ptr<FrameGraphTexture2D> RendererMetal::GetDefaultRT() const
{
    return m_defaultRT;
}

shared_ptr<FrameGraphTexture2D> RendererMetal::GetDefaultDS() const
{
    return m_defaultDS;
}

id<MTLTexture> RendererMetal::GetDefaultDeviceRT()
{
    return m_view.currentRenderPassDescriptor.colorAttachments[0].texture;
}

id<MTLTexture> RendererMetal::GetDefaultDeviceDS()
{
    return m_view.depthStencilTexture;
}

bool RendererMetal::InitWithView(MTKView* view, std::function<bool()> func)
{
    SetupMetal();
    m_view = view;
    m_view.device = m_device;
    m_view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    
    SetupSwapChain();
    
    func();
    
    /// Font stuff
    m_textFont = new FontSegoe_UIW50H12(20);
    m_textRenderPass = new RenderPass(RenderPass::OF_NO_CLEAN,
                                      [&](RenderPassBuilder& builder, PipelineStateObject& pso) {
                                          builder << *m_textFont;
                                          
                                          pso.m_RSState.m_rsState.frontCCW = true;
                                          
                                          // setup render states
                                          auto rsPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("DefaultRT");
                                          pso.m_OMState.m_renderTargetResources[0] = rsPtr;
                                          
                                          auto& target = pso.m_OMState.m_blendState.target[0];
                                          target.enable = true;
                                          target.srcColor = BlendState::Mode::BM_SRC_ALPHA;
                                          target.dstColor = BlendState::Mode::BM_INV_SRC_ALPHA;
                                      },
                                      [&](Renderer& render) {
                                          render.DrawIndexed(m_textFont->GetIndexCount());
                                      });
    
    m_currentFrameGraph = nullptr;
    return true;
}

void RendererMetal::SetupMetal()
{
    // Set the view to use the default device
    m_device = MTLCreateSystemDefaultDevice();
    
    // Create a new command queue
    m_commandQueue = [m_device newCommandQueue];
}

MTLPixelFormat RendererMetal::GetCurrentBackBufferPixelFormat()
{
    return m_view.colorPixelFormat;
}

MTLPixelFormat RendererMetal::GetCurrentDSBufferPixelFormat()
{
    return m_view.depthStencilPixelFormat;
}

id<MTLDevice> RendererMetal::GetDevice()
{
    return m_device;
}

id<MTLCommandBuffer> RendererMetal::CommandBuffer()
{
    id<MTLCommandBuffer> ret = [m_commandQueue commandBuffer];
    ret.label = @"forwardCommandBuffer";
    return ret;
}

RendererMetal::RendererMetal(u32 width, u32 height)
: m_width(width)
, m_height(height)
{}

void RendererMetal::SetupSwapChain()
{
    m_defaultRT = DeviceTextureMetal::BuildDeviceTexture2DDX11("DefaultRT", m_view.currentDrawable.texture);
    if(m_view.depthStencilTexture)
    {
        m_defaultDS = DeviceTextureMetal::BuildDeviceTexture2DDX11("DefaultDS", m_view.depthStencilTexture);
    }
}
