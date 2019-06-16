//
//  ShaderMetal.m
//  HelloMac
//
//  Created by jhq on 2019/5/27.
//

#include "ShaderMetal.h"
#include "render/ShaderSystem/FrameGraphShader.h"
#include "utilities/Utils.h"
#include "macUtil.h"

using namespace forward;

ShaderMetal::ShaderMetal(id<MTLDevice> device, forward::FrameGraphShader* shader)
: DeviceObject(shader)
{
    auto type = shader->GetType();
    switch (type)
    {
        case FGOT_VERTEX_SHADER:
            m_shaderType = VERTEX_SHADER;
            break;
            
        case FGOT_PIXEL_SHADER:
            m_shaderType = PIXEL_SHADER;
            break;
            
        case FGOT_GEOMETRY_SHADER:
            m_shaderType = GEOMETRY_SHADER;
            break;
            
        case FGOT_COMPUTE_SHADER:
            m_shaderType = COMPUTE_SHADER;
            break;
            
        default:
            assert(false);
    }
    
    GenerateShader(device, shader->GetShaderText(), shader->GetShaderEntry());
}

ShaderMetal::~ShaderMetal()
{
}


void ShaderMetal::GenerateShader(id<MTLDevice> device, const std::string& shaderText, const std::wstring& function)
{
    NSError* error = nil;
    MTLCompileOptions *opts = [MTLCompileOptions alloc];
    opts.languageVersion = MTLLanguageVersion2_1;
    opts.fastMathEnabled = NO;
    m_shaderLibrary = [device newLibraryWithSource:CppStringToNSString(shaderText) options:opts error:&error];
    assert(m_shaderLibrary != nil);
    [opts dealloc];
    
    const std::string fileNameAnsi = TextHelper::ToAscii(function);
    m_shaderEntry = [m_shaderLibrary newFunctionWithName:CppStringToNSString(fileNameAnsi)];
}

id<MTLFunction> ShaderMetal::GetEntry()
{
    return m_shaderEntry;
}
