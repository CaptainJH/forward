//
//  DeviceTextureMetal.m
//  BasicGeometryDemo
//
//  Created by jhq on 2019/6/11.
//

#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "DeviceTextureMetal.h"
#include "macUtil.h"

using namespace forward;

shared_ptr<FrameGraphTexture2D> DeviceTextureMetal::BuildDeviceTexture2DDX11(const std::string& name, id<MTLTexture> tex, ResourceUsage usage)
{
    DataFormatType format = ConvertFromMetalPixelFormat(tex.pixelFormat);

    u32 bp = TBP_None;
    if(name == "DefaultRT")
        bp = TBP_RT;
    else if(name == "DefaultDS")
        bp = TBP_DS;
    auto ret = forward::make_shared<FrameGraphTexture2D>(name, format, tex.width, tex.height, bp);
    ret->SetUsage(usage);
    
    return ret;
}

DeviceTextureMetal::DeviceTextureMetal(id<MTLDevice> device, FrameGraphTexture* tex)
: DeviceResourceMetal(tex)
{
    id<MTLTexture> texPtr = nil;
    u32 width, height;
    
    auto type = tex->GetType();
    const auto TBP = tex->GetBindPosition();
    if(type == FrameGraphObjectType::FGOT_TEXTURE2)
    {
        auto tex2D = dynamic_cast<FrameGraphTexture2D*>(tex);
        MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
        texDesc.pixelFormat = Convert2MetalPixelFormat(tex2D->GetFormat());
        texDesc.width = width = tex2D->GetWidth();
        texDesc.height = height = tex2D->GetHeight();
        texDesc.mipmapLevelCount = tex2D->GetMipLevelNum();
        
        if((TBP & TBP_RT) || (TBP & TBP_DS))
        {
            texDesc.usage |= MTLTextureUsageRenderTarget;
        }
        if(TBP & TBP_Shader)
        {
            texDesc.usage |= MTLTextureUsageShaderRead;
        }
//        if(tex->GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL)
//        {
            texDesc.resourceOptions = MTLResourceStorageModeManaged;
//        }
//        else
//        {
//            texDesc.resourceOptions = MTLResourceStorageModePrivate;
//        }
        
        texPtr = [device newTextureWithDescriptor:texDesc];
        m_deviceResPtr = texPtr;
    }
    

    if (tex->GetUsage() == ResourceUsage::RU_IMMUTABLE && (TBP & TBP_Shader) && tex->GetData())
    {
        MTLRegion region = {
            { 0, 0, 0 },       // MTLOrigin
            {width, height, 1} // MTLSize
        };
        
        NSUInteger bytesPerRow = tex->GetElementSize() * width;
        // Copy the bytes from the data object into the texture
        [texPtr replaceRegion:region
                mipmapLevel:0
                withBytes:tex->GetData()
                bytesPerRow:bytesPerRow];
    }
}


id<MTLTexture> DeviceTextureMetal::GetMetalTexturePtr()
{
    id<MTLTexture> ret = (id<MTLTexture>)m_deviceResPtr;
    return ret;
}


void DeviceTextureMetal::SyncCPUToGPU()
{
    
}

void DeviceTextureMetal::SyncGPUToCPU()
{
    auto tex2D = m_frameGraphObjPtr.lock_down<FrameGraphTexture2D>();
    MTLRegion region = MTLRegionMake2D(0, 0, tex2D->GetWidth(), tex2D->GetHeight());
    const i32 bytesPerPixel = DataFormat::GetNumBytesPerStruct(tex2D->GetFormat());
    const i32 bytesPerRow = bytesPerPixel * tex2D->GetWidth();
    u8 *recordingBuffer = tex2D->GetData();
    [GetMetalTexturePtr() getBytes:recordingBuffer bytesPerRow:bytesPerRow fromRegion:region mipmapLevel:0];
}
