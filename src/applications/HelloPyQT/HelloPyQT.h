#pragma once
#include "utilities/Application.h"

extern "C" FORWARD_API void Forward_Constructor(HWND hwnd, int w, int h);
extern "C" FORWARD_API void Forward_Destructor();
extern "C" FORWARD_API void Forward_Update();