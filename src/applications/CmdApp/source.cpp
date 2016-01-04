#include "PCH.h"
#include "GeometryLoader.h"
#include "FileSystem.h"
#include <iostream>

using namespace forward;

i32 main()
{
	FileSystem fileSystem;
	const std::wstring meshFileNameW = L"simpleScene.dae";

	std::vector<GeometryPtr> vGeoms;
	std::vector<Matrix4f> vMats;

	const auto N = GeometryLoader::loadMeshFileDX(meshFileNameW, vGeoms, vMats);

	for (auto i = 0; i < N; ++i)
	{
		auto p = vGeoms[i];
		p->CalculateVertexCount();
		p->CalculateVertexSize();
		std::cout << p->GetVertexCount() << std::endl;
	}

	return 0;
}