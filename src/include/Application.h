//
//  Application.h
//  forward
//
//  Created by jhq on 2019/6/16.
//

#pragma once

#ifdef WINDOWS
#include "ApplicationWin.h"
#elif MACOS
#include "ApplicationMac.h"
#endif


#ifdef WINDOWS
#define FORWARD_APPLICATION_MAIN(CLASS, w, h) \
_Use_decl_annotations_ forward::i32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR cmdLine, i32)\
{\
	CLASS::JustEnteringMain();\
    CLASS app(w, h);\
	app.ParseCmdLine(cmdLine);\
	app.SetAppInst(hInstance);\
    if(!app.Init())\
        return 0;\
    return app.Run();\
}
#elif MACOS
#define FORWARD_APPLICATION_MAIN(CLASS, w, h) \
int main(int argc, const char * argv[])\
{\
    CLASS app(w, h);\
    return app.Run();\
}
#endif
