//
//  macUtil.m
//  HelloMac
//
//  Created by jhq on 2019/5/29.
//

#include "macUtil.h"


NSString* forward::CppStringToNSString(const std::string& strIn)
{
    NSString* ret = [NSString stringWithCString:strIn.c_str() encoding:[NSString defaultCStringEncoding]];
    return ret;
}

std::string forward::NSStringToCppString(NSString* strIn)
{
    std::string ret = [strIn UTF8String];
    return ret;
}


MTLVertexFormat forward::Convert2MetalVertexFormat(const DataFormatType type)
{
    switch(type)
    {
        case DataFormatType::DF_R32G32B32_FLOAT:
            return MTLVertexFormatFloat3;
            
        case DataFormatType::DF_R32G32B32A32_FLOAT:
            return MTLVertexFormatFloat4;
            
        case DataFormatType::DF_R32G32_FLOAT:
            return MTLVertexFormatFloat2;
    };
    
    assert(false && "Should not run to here!");
    return MTLVertexFormatInvalid;
}

MTLPixelFormat forward::Convert2MetalPixelFormat(const DataFormatType type)
{
    switch(type)
    {
        case DF_R8_UNORM:
            return MTLPixelFormatR8Unorm;
            
        case DF_R8G8B8A8_UNORM:
            return MTLPixelFormatRGBA8Unorm;
    };
    assert(false && "Should not run to here!");
    return MTLPixelFormatInvalid;
}

forward::DataFormatType forward::ConvertFromMetalPixelFormat(const MTLPixelFormat type)
{
    switch(type)
    {
        case MTLPixelFormatR8Unorm:
            return DF_R8_UNORM;
            
        case MTLPixelFormatBGRA8Unorm:
            return DF_B8G8R8A8_UNORM;
            
        case MTLPixelFormatDepth32Float_Stencil8:
            return DF_D32_FLOAT;
            
    };
    assert(false && "Should not run to here!");
    return DF_UNKNOWN;
}
