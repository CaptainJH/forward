#pragma once
#include <math.h>
#include "FrameGraph/Geometry.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "Matrix4f.h"
#include "utilities/Utils.h"

#include <ImathQuat.h>

namespace forward
{
	struct LoadedResourceManager;
	struct SceneData
	{
		struct Instance
		{
			String name;
			i32 meshId;

			float3 translation;
			float3 scale;
			Imath::Quatf rotation;
			float4x4 mat;
		};

		static SceneData LoadFromFile(const std::wstring fileName, LoadedResourceManager& resMgr);
		static SceneData LoadFromFileForStandSurface(const std::wstring fileName, LoadedResourceManager& resMgr);

		Vector<SimpleGeometry> mMeshData;
		Vector<Instance> mInstances;
	};
}