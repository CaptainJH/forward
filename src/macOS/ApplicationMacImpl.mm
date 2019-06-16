//
//  ApplicationMacImpl.cpp
//  forwardMetal
//
//  Created by jhq on 2019/5/22.
//

#include "PCH.h"
#include "ApplicationMacImpl.h"
#include "ApplicationMac.h"
#include "RendererMetal.h"
#import <Cocoa/Cocoa.h>

using namespace forward;

ApplicationMacImpl::ApplicationMacImpl(ApplicationMac* app)
: m_pApp(app)
{
    m_pRender = new RendererMetal(GetWindowWidth(), GetWindowHeight());
    m_pApp->m_pRender = m_pRender;
    gContext.Application = this;
}

ApplicationMacImpl::~ApplicationMacImpl()
{
    SAFE_DELETE(m_pRender);
}

int ApplicationMacImpl::NSApplicationMainWrapper()
{
    return NSApplicationMain(0, nullptr);
}

void ApplicationMacImpl::InitWithView(void* mtkView)
{
    m_pRender->InitWithView((MTKView*)mtkView, [&]()->bool{
        return m_pApp->Init();
    });
}

void ApplicationMacImpl::Render()
{
    assert(m_pApp);
    m_pApp->mTimer.Tick();
    m_pApp->UpdateScene(0.0f);
    m_pApp->DrawScene();
}

int ApplicationMacImpl::GetWindowWidth() const
{
    return m_pApp->m_ClientWidth;
}

int ApplicationMacImpl::GetWindowHeight() const
{
    return m_pApp->m_ClientHeight;
}

void ApplicationMacImpl::OnInput(unsigned short input)
{
    m_pApp->OnInput(input);
}


ApplicationMacImpl::ApplicationContext ApplicationMacImpl::gContext;
