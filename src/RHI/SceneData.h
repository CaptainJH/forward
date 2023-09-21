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

		struct MaterialData
		{
			String	name;

			Color3	baseColor;
			i32		baseColorTexIdx = -1;
			String	baseColorTexName;

			Color3	emissive;
			i32		emissiveTexIdx = -1;
			String	emissiveTexName;

			String	ambientTexName;

			f32		metalness = -1.0f;
			f32		roughness = -1.0f;
			f32		opacity;
			i32		roughnessMetalnessTexIdx = -1;
			String	roughnessMetalnessTexName;

			i32		alphaMode;                  //< 0: Opaque, 1: Blend, 2: Masked
			f32		alphaCutoff;
			i32		doubleSided = 0;                //< 0: false, 1: true
			i32		normalTexIdx = -1;               //< Tangent space XYZ
			String	normalTexName;
		};

		static SceneData LoadFromFile(const std::wstring fileName, LoadedResourceManager& resMgr);
		static SceneData LoadFromFileForStandSurface(const std::wstring fileName, LoadedResourceManager& resMgr);

		Vector<SimpleGeometry> mMeshData;
		Vector<Instance> mInstances;
		Vector<MaterialData> mMaterials;
	};
}