#pragma once
#include "FrameGraph/Geometry.h"
#include <ImathQuat.h>

namespace forward
{
	struct LoadedResourceManager;
	struct SceneData
	{
		struct Instance
		{
			String name;
			u32 meshId;
			u32 materialId;

			float3 translation;
			float3 scale;
			Imath::Quatf rotation;
			float4x4 mat;
		};

		struct MaterialData
		{
			float3	baseColor;
			i32		baseColorTexIdx = -1;

			float3	emissive;
			i32		emissiveTexIdx = -1;

			f32		metalness = -1.0f;
			f32		roughness = -1.0f;
			f32		opacity;
			i32		roughnessMetalnessTexIdx = -1;

			i32		alphaMode;                  //< 0: Opaque, 1: Blend, 2: Masked
			f32		alphaCutoff;
			i32		doubleSided = 0;                //< 0: false, 1: true
			i32		normalTexIdx = -1;               //< Tangent space XYZ
		};

		struct MaterialDesc
		{
			String	name;
			String	baseColorTexName;
			String	emissiveTexName;
			String	ambientTexName;
			String	roughnessMetalnessTexName;
			String	normalTexName;

			MaterialData materialData;
		};

		static SceneData LoadFromFile(const std::wstring fileName, LoadedResourceManager& resMgr);
		static SceneData LoadFromFileForStandSurface(const std::wstring fileName, LoadedResourceManager& resMgr);

		Vector<SimpleGeometry> mMeshData;
		Vector<Instance> mInstances;
		Vector<MaterialDesc> mMaterials;
		Vector<shared_ptr<Texture2D>> mTextures;
	};
}