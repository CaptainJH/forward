//
//  macUtil.h
//  forward
//
//  Created by jhq on 2019/5/29.
//

#pragma once
#include "PCH.h"
#import <Metal/Metal.h>
#include "render/DataFormat.h"


namespace forward
{
    NSString* CppStringToNSString(const std::string& strIn);
    std::string NSStringToCppString(NSString* strIn);
    
    MTLVertexFormat Convert2MetalVertexFormat(const DataFormatType type);
    MTLPixelFormat Convert2MetalPixelFormat(const DataFormatType type);
    DataFormatType ConvertFromMetalPixelFormat(const MTLPixelFormat type);
}
