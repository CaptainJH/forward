//
//  ShaderMetal.h
//  forward
//
//  Created by jhq on 2019/5/27.
//

#pragma once
#include "PCH.h"
#include "render/ResourceSystem/DeviceObject.h"
#import <Metal/Metal.h>

namespace forward
{
    class FrameGraphShader;
    
    enum ShaderType
    {
        VERTEX_SHADER = 0,
        HULL_SHADER = 1,
        DOMAIN_SHADER = 2,
        GEOMETRY_SHADER = 3,
        PIXEL_SHADER = 4,
        COMPUTE_SHADER = 5
    };
    
    
    enum ShaderMask
    {
        VERTEX_SHADER_MSK = 0x0001,
        HULL_SHADER_MSK = 0x0002,
        DOMAIN_SHADER_MSK = 0x0004,
        GEOMETRY_SHADER_MSK = 0x0008,
        PIXEL_SHADER_MSK = 0x0010,
        COMPUTE_SHADER_MSK = 0x0020
    };
    
    class ShaderMetal : public DeviceObject
    {
    public:
        ShaderMetal(id<MTLDevice> device, forward::FrameGraphShader* shader);
        virtual ~ShaderMetal();
        
        ShaderType GetType() const;
        
        id<MTLFunction> GetEntry();
        
    private:
        id<MTLLibrary> m_shaderLibrary;
        id<MTLFunction> m_shaderEntry;
        
        ShaderType      m_shaderType;
        
    private:
        void GenerateShader(id<MTLDevice> device, const std::string& shaderText, const std::wstring& function);
    };
    
}
