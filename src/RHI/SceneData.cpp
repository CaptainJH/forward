#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <assimp/GltfMaterial.h>

#include "SceneData.h"
#include "FileSystem.h"
#include "Log.h"
#include "RHI/Device.h"

using namespace forward;

const i32 ALPHA_MODE_OPAQUE = 0;
const i32 ALPHA_MODE_BLEND = 1;
const i32 ALPHA_MODE_MASK = 2;

SceneData SceneData::LoadFromFile(const std::wstring fileName, LoadedResourceManager& resMgr)
{
	SceneData ret;

	WString sceneFilePathW = FileSystem::getSingleton().GetModelsFolder() + fileName;
	if (!FileSystem::getSingleton().FileExists(sceneFilePathW))
		sceneFilePathW = FileSystem::getSingleton().GetExternFolder() + fileName;
	assert(FileSystem::getSingleton().FileExists(sceneFilePathW));
	const String sceneFileFullPath = TextHelper::ToAscii(sceneFilePathW);

	const aiScene* scene = aiImportFile(sceneFileFullPath.c_str(), aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || !scene->HasMeshes())
	{
		std::wstringstream wss;
		wss << fileName << L" loading failed!";
		auto text = wss.str();
		Log::Get().Write(text);
		assert(false);
	}

	for (auto idx = 0U; idx < scene->mNumMeshes; ++idx)
	{
		const aiMesh* mesh = scene->mMeshes[idx];
		auto vf = Vertex_POS_UV::GetVertexFormat();
		auto meshFullName = TextHelper::ToAscii(fileName) + ":" + mesh->mName.C_Str();
		std::stringstream ss;
		ss << idx << " : " << meshFullName << std::endl;
		OutputDebugStringA(ss.str().c_str());
		auto loadedVB = shared_ptr<VertexBuffer>(dynamic_cast<VertexBuffer*>(resMgr.FindVertexBufferByName(meshFullName).get()));
		auto loadedIB = shared_ptr<IndexBuffer>(dynamic_cast<IndexBuffer*>(resMgr.FindIndexBufferByName(meshFullName).get()));
		if (loadedVB && loadedIB)
			ret.mMeshData.push_back(SimpleGeometry(loadedVB, loadedIB));
		else
		{
			SimpleGeometry sgeo(meshFullName, vf, PrimitiveTopologyType::PT_TRIANGLELIST,
				mesh->mNumVertices, mesh->mNumFaces * 3);

			for (auto i = 0U; i != mesh->mNumVertices; i++)
			{
				const aiVector3D v = mesh->mVertices[i];
				const aiVector3D t = mesh->mTextureCoords[0] ?
					mesh->mTextureCoords[0][i] : aiVector3D{ 0.0f, 0.0f, 0.0f };
				sgeo.AddVertex<Vertex_POS_UV>({ .Pos = Vector3f(v.x, v.y, v.z), 
					.UV = Vector2f(t.x > 0.0f ? t.x : -t.x, 
						t.y > 0.0f ? t.y : -t.y) });
			}

			for (auto i = 0U; i != mesh->mNumFaces; i++)
			{
				assert(mesh->mFaces[i].mNumIndices == 3);
				sgeo.AddFace(TriangleIndices(mesh->mFaces[i].mIndices[0],
					mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]));
			}

			resMgr.mAllLoadedBuffers.push_back(sgeo.m_VB);
			resMgr.mAllLoadedBuffers.push_back(sgeo.m_IB);
			ret.mMeshData.push_back(sgeo);
		}
	}

	auto& rootNode = *scene->mRootNode;
	if (rootNode.mNumMeshes > 0)
	{
		aiVector3D scale, pos;
		aiQuaternion rot;
		rootNode.mTransformation.Decompose(scale, rot, pos);
		float4x4 mT;
		mT.setTranslation(float3(pos.x, pos.y, pos.z));
		float4x4 mS;
		mS.setScale(float3(scale.x, scale.y, scale.z));
		Imath::Quatf q(rot.w, rot.x, rot.y, rot.z);
		auto mRot = q.toMatrix44();

		ret.mInstances.push_back({
			.name = rootNode.mName.C_Str(),
			.meshId = static_cast<i32>(rootNode.mMeshes[0]),
			.translation = {pos.x, pos.y, pos.z},
			.scale = {scale.x, scale.y, scale.z},
			.rotation = {rot.w, rot.x, rot.y, rot.z},
			.mat = mRot * mS * mT
			});
	}

	for (auto idx = 0U; idx < scene->mRootNode->mNumChildren; ++idx)
	{
		auto node = scene->mRootNode->mChildren[idx];
		if (node->mNumMeshes == 0)
			continue;
		aiVector3D scale, pos;
		aiQuaternion rot;
		node->mTransformation.Decompose(scale, rot, pos);
		float4x4 mT;
		mT.setTranslation(float3(pos.x, pos.y, pos.z));
		float4x4 mS;
		mS.setScale(float3(scale.x, scale.y, scale.z));
		Imath::Quatf q(rot.w, rot.x, rot.y, rot.z);
		auto mRot = q.toMatrix44();

		ret.mInstances.push_back({
			.name = node->mName.C_Str(),
			.meshId = static_cast<i32>(node->mMeshes[0]),
			.translation = {pos.x, pos.y, pos.z},
			.scale = {scale.x, scale.y, scale.z},
			.rotation = {rot.w, rot.x, rot.y, rot.z},
			.mat = mRot * mS * mT
			});
	}

	for (auto idx = 0U; idx < scene->mNumMaterials; ++idx)
	{
		const aiMaterial* mat = scene->mMaterials[idx];

		MaterialData materialData;
		materialData.name = mat->GetName().C_Str();

		aiColor4D Color;
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &Color) == AI_SUCCESS)
		{
			assert(false);
		}
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &Color) == AI_SUCCESS)
		{
			materialData.baseColor = { Color.r, Color.g, Color.b };
			materialData.opacity = Color.a;
		}
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &Color) == AI_SUCCESS)
		{
			materialData.emissive = { Color.r, Color.g, Color.b };
		}

		int twoSided = 0;
		if (aiGetMaterialInteger(mat, AI_MATKEY_TWOSIDED, &twoSided) == AI_SUCCESS)
		{
			materialData.doubleSided = twoSided;
		}

		f32 Opacity = 1.0f;
		if (aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &Opacity) == AI_SUCCESS)
		{
			materialData.opacity = Opacity;
		}

		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_TRANSPARENT, &Color) == AI_SUCCESS)
		{
			assert(false);
		}

		f32 metallicFactor = -1.0f;
		if (aiGetMaterialFloat(mat, AI_MATKEY_METALLIC_FACTOR, &metallicFactor) == AI_SUCCESS)
		{
			materialData.metalness = metallicFactor;
		}

		f32 roughnessFactor = -1.0f;
		if (aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR, &roughnessFactor) == AI_SUCCESS)
		{
			materialData.roughness = roughnessFactor;
		}

		aiString alphaMode;
		if (aiGetMaterialString(mat, AI_MATKEY_GLTF_ALPHAMODE, &alphaMode) == AI_SUCCESS)
		{
			if (strcmp(alphaMode.C_Str(), "OPAQUE") == 0) materialData.alphaMode = ALPHA_MODE_OPAQUE;
			else if (strcmp(alphaMode.C_Str(), "BLEND") == 0) materialData.alphaMode = ALPHA_MODE_BLEND;
			else if (strcmp(alphaMode.C_Str(), "MASK") == 0) materialData.alphaMode = ALPHA_MODE_MASK;
		}

		f32 alphaCutoff;
		if (aiGetMaterialFloat(mat, AI_MATKEY_GLTF_ALPHACUTOFF, &alphaCutoff) == AI_SUCCESS)
		{
			materialData.alphaCutoff = alphaCutoff;
		}

		aiString Path;
		aiTextureMapping Mapping;
		unsigned int UVIndex = 0;
		f32 Blend = 1.0f;
		aiTextureOp TextureOp = aiTextureOp_Add;
		aiTextureMapMode TextureMapMode[2] = { aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };
		unsigned int TextureFlags = 0;

		if (aiGetMaterialTexture(mat, aiTextureType_EMISSIVE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialData.emissiveTexName = Path.C_Str();
		}

		if (aiGetMaterialTexture(mat, aiTextureType_DIFFUSE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialData.baseColorTexName = Path.C_Str();
		}

		// first try tangent space normal map
		if (aiGetMaterialTexture(mat, aiTextureType_NORMALS, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialData.normalTexName = Path.C_Str();
		}

		if (aiGetMaterialTexture(mat, aiTextureType_LIGHTMAP, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialData.ambientTexName = Path.C_Str();
		}

		if (aiGetMaterialTexture(mat, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialData.roughnessMetalnessTexName = Path.C_Str();
		}
		//// then height map
		//	if (aiGetMaterialTexture(M, aiTextureType_HEIGHT, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		//		D.normalMap_ = addUnique(files, Path.C_Str());

		//if (aiGetMaterialTexture(M, aiTextureType_OPACITY, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		//{
		//	D.opacityMap_ = addUnique(opacityMaps, Path.C_Str());
		//	D.alphaTest_ = 0.5f;
		//}

		// patch materials
		aiString Name;
		std::string materialName;
		if (aiGetMaterialString(mat, AI_MATKEY_NAME, &Name) == AI_SUCCESS)
		{
			materialName = Name.C_Str();
			assert(materialName == materialData.name);
		}

		ret.mMaterials.push_back(materialData);
	}

	aiReleaseImport(scene);
	return ret;
}

SceneData SceneData::LoadFromFileForStandSurface(const std::wstring fileName, LoadedResourceManager& resMgr)
{
	SceneData ret;

	WString sceneFilePathW = FileSystem::getSingleton().GetModelsFolder() + fileName;
	if (!FileSystem::getSingleton().FileExists(sceneFilePathW))
		sceneFilePathW = FileSystem::getSingleton().GetExternFolder() + fileName;
	assert(FileSystem::getSingleton().FileExists(sceneFilePathW));
	const String sceneFileFullPath = TextHelper::ToAscii(sceneFilePathW);

	const aiScene* scene = aiImportFile(sceneFileFullPath.c_str(), aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || !scene->HasMeshes())
	{
		std::wstringstream wss;
		wss << fileName << L" loading failed!";
		auto text = wss.str();
		Log::Get().Write(text);
		assert(false);
	}

	for (auto idx = 0U; idx < scene->mNumMeshes; ++idx)
	{
		const aiMesh* mesh = scene->mMeshes[idx];
		auto vf = Vertex_P_N_T_UV::GetVertexFormat();
		auto meshFullName = TextHelper::ToAscii(fileName) + ":" + mesh->mName.C_Str();
		auto loadedVB = shared_ptr<VertexBuffer>(dynamic_cast<VertexBuffer*>(resMgr.FindVertexBufferByName(meshFullName).get()));
		auto loadedIB = shared_ptr<IndexBuffer>(dynamic_cast<IndexBuffer*>(resMgr.FindIndexBufferByName(meshFullName).get()));
		if (loadedVB && loadedIB)
			ret.mMeshData.push_back(SimpleGeometry(loadedVB, loadedIB));
		else
		{
			SimpleGeometry sgeo(meshFullName, vf, PrimitiveTopologyType::PT_TRIANGLELIST,
				mesh->mNumVertices, mesh->mNumFaces * 3);

			for (auto i = 0U; i != mesh->mNumVertices; i++)
			{
				const aiVector3D v = mesh->mVertices[i];
				const aiVector3D n = mesh->mNormals[i];
				const aiVector3D t = mesh->mTangents[i];
				const aiVector3D uv = mesh->mTextureCoords[0][i];
				sgeo.AddVertex<Vertex_P_N_T_UV>({
					.Pos = Vector3f(v.x, v.y, v.z),
					.Normal = Vector3f(n.x, n.y, n.z), 
					.Tangent = Vector3f(t.x, t.y, t.z),
					.UV = Vector2f(uv.x, uv.y)
					});
			}

			for (auto i = 0U; i != mesh->mNumFaces; i++)
			{
				assert(mesh->mFaces[i].mNumIndices == 3);
				sgeo.AddFace(TriangleIndices(mesh->mFaces[i].mIndices[0],
					mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]));
			}

			resMgr.mAllLoadedBuffers.push_back(sgeo.m_VB);
			resMgr.mAllLoadedBuffers.push_back(sgeo.m_IB);
			ret.mMeshData.push_back(sgeo);
		}
	}

	aiReleaseImport(scene);
	return ret;
}

