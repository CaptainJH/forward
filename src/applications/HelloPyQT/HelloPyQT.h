#pragma once
#include "ApplicationWin.h"


#pragma once
#ifdef FORWARD_EXPORTS
#define FORWARD_API __declspec(dllexport) 
#else
#define FORWARD_API __declspec(dllimport) 
#endif

extern "C" FORWARD_API void Forward_Constructor(HWND hwnd, int w, int h);
extern "C" FORWARD_API void Forward_Destructor();
extern "C" FORWARD_API void Forward_Update();