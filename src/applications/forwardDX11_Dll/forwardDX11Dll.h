#pragma once
#ifdef FORWARD_EXPORTS
#define FORWARD_API __declspec(dllexport) 
#else
#define FORWARD_API __declspec(dllimport) 
#endif

extern "C" FORWARD_API void Forward_Constructor();
extern "C" FORWARD_API void Forward_Destructor();

extern "C" FORWARD_API const wchar_t* FileSystem_GetFontFolder();