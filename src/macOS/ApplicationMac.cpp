//
//  ApplicationMac.cpp
//  forwardMetal
//
//  Created by jhq on 2019/5/18.
//

#include "ApplicationMac.h"
#include "ApplicationMacImpl.h"

using namespace forward;

ApplicationMac::ApplicationMac(i32 width, i32 height)
: m_ClientWidth(width)
, m_ClientHeight(height)
{
    m_Impl = new ApplicationMacImpl(this);
}

ApplicationMac::~ApplicationMac()
{
    SAFE_DELETE(m_Impl);
}

i32 ApplicationMac::Run()
{
    return m_Impl->NSApplicationMainWrapper();
}

bool ApplicationMac::Init()
{
    return true;
}

f32 ApplicationMac::AspectRatio() const
{
    return static_cast<f32>(m_ClientWidth) / m_ClientHeight;
}
