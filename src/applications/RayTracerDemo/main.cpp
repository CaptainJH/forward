#include "PCH.h"
#include "Timer.h"
#include "FileSystem.h"
#include "Log.h"
#include "Utils.h"

#include "Vector2f.h"
#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix4f.h"

#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"
#include <iostream>

using namespace forward;

forward::FileSystem g_fileSystem;

i32 main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

}