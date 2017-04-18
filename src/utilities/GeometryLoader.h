//--------------------------------------------------------------------------------
// GeometryLoaderDX11
//
//--------------------------------------------------------------------------------
#pragma once
//--------------------------------------------------------------------------------
#include "Pipeline\Executors\GeometryDX11.h"
#include "Matrix4f.h"
#include <vector>
#include <string>
//--------------------------------------------------------------------------------
namespace forward
{
	class GeometryLoader
	{
	public:
		static i32 loadMeshFileDX( std::wstring filename, std::vector<GeometryPtr>& outMeshes, std::vector<Matrix4f>& outMtx );		
		

	private:
		GeometryLoader();
	};
};