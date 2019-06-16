//
//  DeviceDrawingStateMetal.m
//  HelloMac
//
//  Created by jhq on 2019/6/9.
//

#include "DeviceDrawingStateMetal.h"

using namespace forward;

DeviceDepthStencilStateMetal::DeviceDepthStencilStateMetal(id<MTLDevice> device, DepthStencilState* ds)
: DeviceObject(ds)
{
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    m_deviceState = [device newDepthStencilStateWithDescriptor:depthStateDesc];
}

void DeviceDepthStencilStateMetal::Bind(id<MTLRenderCommandEncoder> encoder)
{
    [encoder setDepthStencilState:m_deviceState];
}
