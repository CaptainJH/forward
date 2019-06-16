//
//  ApplicationMac.hpp
//  forwardMetal
//
//  Created by jhq on 2019/5/18.
//

#pragma once

#include "PCH.h"
#include "Timer.h"
#include "FileSystem.h"
#include "Log.h"

#include "Vector2f.h"
#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix4f.h"

#include "render.h"
#include "Utils.h"

namespace forward
{
    class ApplicationMacImpl;
    class Renderer;
    
    class ApplicationMac
    {
    public:
        ApplicationMac(i32 width, i32 height);
        ~ApplicationMac();
        
        f32 AspectRatio() const;
        i32 Run();
        
        virtual bool Init();
        virtual void OnResize();
        
    protected:
        virtual void UpdateScene(f32 dt) = 0;
        virtual void DrawScene() = 0;
        
        virtual void OnInput(u16 input) {}
        
    protected:
        
        i32 m_ClientWidth;
        i32 m_ClientHeight;
        std::wstring mMainWndCaption;
        
        Timer mTimer;
        FileSystem mFileSystem;
        
        Renderer* m_pRender2 = nullptr;
        
        ApplicationMacImpl* m_Impl = nullptr;
        friend class ApplicationMacImpl;
        
    };
    
    typedef ApplicationMac Application;
}
