//
//  RendererMetal.h
//  forward
//
//  Created by jhq on 2019/5/22.
//

#pragma once
#include <functional>
#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>
#include "render/render.h"
#include "render/FrameGraph/RenderPass.h"
#include "render/ShaderSystem/FrameGraphShader.h"
#include "render/Text/FontSegoe_UIW50H12.h"

namespace forward
{
    class RendererMetal : public Renderer
    {
    public:
        virtual ~RendererMetal();
        RendererAPI GetRendererAPI() const override;
        
        void DrawRenderPass(RenderPass& pass) override;
        
        void DeleteResource(ResourcePtr ptr) override;
        
        void OnResize(u32 width, u32 height) override;
        
        bool Initialize(SwapChainConfig& config, bool bOffScreen) override;
        void Shutdown() override;
        
        void Draw(u32 vertexNum, u32 startVertexLocation=0) override;
        void DrawIndexed(u32 indexCount) override;
        
        void ResolveResource(FrameGraphTexture2D* dst, FrameGraphTexture2D* src) override;
        
        void SaveRenderTarget(const std::wstring& filename, PipelineStateObject& pso) override;
        
        void DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color) override;
        
        void EndDrawFrameGraph() override;
        
        shared_ptr<FrameGraphTexture2D> GetDefaultRT() const override;
        shared_ptr<FrameGraphTexture2D> GetDefaultDS() const override;
        
        id<MTLTexture> GetDefaultDeviceRT();
        id<MTLTexture> GetDefaultDeviceDS();
        
        bool InitWithView(MTKView* view, std::function<bool()> func);
        
        MTLPixelFormat GetCurrentBackBufferPixelFormat();
        MTLPixelFormat GetCurrentDSBufferPixelFormat();
        
        id<MTLDevice> GetDevice();
        id<MTLCommandBuffer> CommandBuffer();
        
        RendererMetal(u32 width, u32 height);
        
    private:
        void SetupMetal();
        void SetupSwapChain();
        
        
    private:
        id<MTLDevice> m_device;
        id<MTLCommandQueue> m_commandQueue;
        
        // view
        MTKView *m_view;
        
        id<MTLCommandBuffer> m_currentCommandBuffer;
        id<MTLRenderCommandEncoder> m_currentEncoder;
        id<MTLBuffer>  m_currentIndexBuffer;
        RenderPass* m_currentPass;
        MTLPrimitiveType m_currentPrimitiveType;
        
        // controller
        dispatch_semaphore_t _inflight_semaphore;
        
        u32 m_width;
        u32 m_height;
        
        Font*    m_textFont;
        RenderPass* m_textRenderPass;
        
        shared_ptr<FrameGraphTexture2D> m_defaultRT;
        shared_ptr<FrameGraphTexture2D> m_defaultDS;
    };
}
