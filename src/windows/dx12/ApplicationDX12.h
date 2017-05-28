//--------------------------------------------------------------------------------
// Application
//
// This class represents the base application available to the end user.  The 
// Windows Main function is contained withing the .cpp file, and automatically
// checks for an instance of a CApplication class.  If one is not found then the
// program is exited.
//
// The application currently supports Input, Sound, Rendering, Logging, Timing, 
// and profiling.  These are all available to the user when building an 
// application.
//
//--------------------------------------------------------------------------------
#pragma once

//--------------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "PCH.h"
#include "d3dUtil.h"
//#include "RendererDX11.h"
#include "Timer.h"
#include "FileSystem.h"
#include "Log.h"
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
namespace forward
{
	class ApplicationDX12
	{
	public:
	};

};
//--------------------------------------------------------------------------------