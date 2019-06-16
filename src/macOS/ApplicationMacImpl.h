//
//  ApplicationMacImpl.hpp
//  forwardMetal
//
//  Created by jhq on 2019/5/22.
//

#pragma once

namespace forward
{
    class RendererMetal;
    class ApplicationMac;
    
    
    class ApplicationMacImpl
    {
    public:
        ApplicationMacImpl(ApplicationMac* app);
        ~ApplicationMacImpl();
        
        void InitWithView(void* mtkView);
        void Render();
        
        int NSApplicationMainWrapper();
        
        int GetWindowWidth() const;
        int GetWindowHeight() const;
        
        void OnInput(unsigned short input);
        
        struct ApplicationContext
        {
            ApplicationMacImpl* Application = nullptr;
        };
        
        static ApplicationContext gContext;
        
    private:
        RendererMetal* m_pRender = nullptr;
        ApplicationMac* m_pApp = nullptr;
        
    };
}
