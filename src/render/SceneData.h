#pragma once
#include <math.h>
#include "FrameGraph/Geometry.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "utilities/Utils.h"

namespace forward
{
	struct SceneData
	{
		static SceneData LoadFromFile(const std::wstring fileName);

		std::vector<SimpleGeometry> mMeshData;
	};
}