#include "PCH.h"
#include "Timer.h"
#include "FileSystem.h"
#include "FileSaver.h"
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

const i32 Width = 1600;
const i32 Height = 900;

i32 main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	u8* imageBuffer = new u8[Width * Height * 4];
	u8* currentPixel = nullptr;
	for (auto h = 0; h < Height; ++h)
	{
		for (auto w = 0; w < Width; ++w)
		{
			currentPixel = imageBuffer + h * Width * 4 + w * 4;
			*(u32*)currentPixel = Colors::Blue.toARGB();
		}
	}

	FileSaver outputFile;
	outputFile.SaveAsBMP(L"RayTracerOutput.bmp", imageBuffer, Width, Height);
	SAFE_DELETE_ARRAY(imageBuffer);
}