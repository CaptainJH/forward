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

#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"
#include "intersection/IntrRay3fSphere3f.h"
#include <iostream>

using namespace forward;

forward::FileSystem g_fileSystem;

const i32 Width = 1000;
const i32 Height = 1000;

i32 main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Build the view matrix.
	Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	auto viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);

	u8* imageBuffer = new u8[Width * Height * 4];
	u8* currentPixel = nullptr;

	Vector4f sphere_center_viewspace = viewMat * Vector4f(target, 1.0f);
	Sphere3f sphere(sphere_center_viewspace.xyz(), 1.0f);

	const float step_w = 2.0f / Width;
	const float step_h = 2.0f / Height;
	Vector3f current_pos(-1.0f, 1.0f, 1.0f);
	for (auto h = 0; h < Height; ++h)
	{
		for (auto w = 0; w < Width; ++w)
		{
			current_pos.x += step_w;
			Vector3f current_dir = current_pos;
			current_dir.Normalize();
			Vector3f origin; origin.MakeZero();
			Ray3f ray(origin, current_dir);
			IntrRay3fSphere3f inter(ray, sphere);

			currentPixel = imageBuffer + h * Width * 4 + w * 4;
			if (inter.Test())
			{
				*(u32*)currentPixel = Colors::Red.toARGB();
			}
			else
			{
				*(u32*)currentPixel = Colors::Blue.toARGB();
			}
		}

		current_pos.y -= step_h;
		current_pos.x = -1.0f;
	}

	FileSaver outputFile;
	outputFile.SaveAsBMP(L"RayTracerOutput.bmp", imageBuffer, Width, Height);
	SAFE_DELETE_ARRAY(imageBuffer);
}