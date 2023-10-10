#include <filesystem>
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

	String sceneFileFullPath = TextHelper::ToAscii(fileName);
	WString sceneFilePathW = fileName;
	if (!FileSystem::getSingleton().FileExists(sceneFilePathW))
	{
		sceneFilePathW = FileSystem::getSingleton().GetModelsFolder() + fileName;
		if (!FileSystem::getSingleton().FileExists(sceneFilePathW))
			sceneFilePathW = FileSystem::getSingleton().GetExternFolder() + fileName;
		assert(FileSystem::getSingleton().FileExists(sceneFilePathW));
		sceneFileFullPath = TextHelper::ToAscii(sceneFilePathW);
	}

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
				const aiVector3D zero = { 0.0f, 0.0f, 0.0f };
				const aiVector3D v = mesh->mVertices[i];
				const aiVector3D n = mesh->mNormals ? mesh->mNormals[i] : zero;
				const aiVector3D t = mesh->mTangents ? mesh->mTangents[i] : zero;
				const aiVector3D uv = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : zero;
				sgeo.AddVertex<Vertex_P_N_T_UV>({
					.Pos = Vector3f(v.x, v.y, v.z), 
					.Normal = Vector3f(n.x, n.y, n.z),
					.Tangent = Vector3f(t.x, t.y, t.z),
					.UV = Vector2f(std::abs(uv.x), std::abs(uv.y)) 
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

	std::function<void(aiNode&, float4x4)> AddInstance = [&](aiNode& node, float4x4 parentMat) {
		aiVector3D scale, pos;
		aiQuaternion rot;
		node.mTransformation.Decompose(scale, rot, pos);
		float4x4 mT;
		mT.setTranslation(float3(pos.x, pos.y, pos.z));
		float4x4 mS;
		mS.setScale(float3(scale.x, scale.y, scale.z));
		Imath::Quatf q(rot.w, rot.x, rot.y, rot.z);
		auto mRot = q.toMatrix44();
		auto currentTransformMat = mRot * mS * mT * parentMat;

		for (auto i = 0U; i < node.mNumMeshes; ++i)
			ret.mInstances.push_back({
				.name = node.mName.C_Str(),
				.meshId = node.mMeshes[i],
				.materialId = scene->mMeshes[node.mMeshes[i]]->mMaterialIndex,
				.translation = {pos.x, pos.y, pos.z},
				.scale = {scale.x, scale.y, scale.z},
				.rotation = {rot.w, rot.x, rot.y, rot.z},
				.mat = currentTransformMat
				});
		for (auto i = 0U; i < node.mNumChildren; ++i)
			AddInstance(*node.mChildren[i], currentTransformMat);
		};

	float4x4 instMat; instMat.makeIdentity();
	auto& rootNode = *scene->mRootNode;
	AddInstance(rootNode, instMat);

	auto FindTextureId = [&](const String& n)->u32 {
		std::filesystem::path sceneFile = sceneFilePathW;
		auto texFullPath = sceneFile.parent_path().append(n);
		assert(FileSystem::getSingleton().FileExists(texFullPath));
		const auto it = std::find_if(ret.mTextures.begin(), ret.mTextures.end(), [&](auto& p)->bool {
			return p->Name() == n;
			});
		if (it == ret.mTextures.end())
		{
			ret.mTextures.emplace_back(forward::make_shared<Texture2D>(n, texFullPath.c_str()));
			resMgr.mAllLoadedTextures.push_back(ret.mTextures.back());
			return static_cast<u32>(ret.mTextures.size() - 1);
		}
		else
			return static_cast<u32>(it - ret.mTextures.begin());
		};

	for (auto idx = 0U; idx < scene->mNumMaterials; ++idx)
	{
		const aiMaterial* mat = scene->mMaterials[idx];

		MaterialDesc materialDesc;
		materialDesc.name = mat->GetName().C_Str();

		aiColor4D Color;
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &Color) == AI_SUCCESS)
		{
			assert(false);
		}
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &Color) == AI_SUCCESS)
		{
			materialDesc.materialData.baseColor = { Color.r, Color.g, Color.b };
			materialDesc.materialData.opacity = Color.a;
		}
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &Color) == AI_SUCCESS)
		{
			materialDesc.materialData.emissive = { Color.r, Color.g, Color.b };
		}

		int twoSided = 0;
		if (aiGetMaterialInteger(mat, AI_MATKEY_TWOSIDED, &twoSided) == AI_SUCCESS)
		{
			materialDesc.materialData.doubleSided = twoSided;
		}

		f32 Opacity = 1.0f;
		if (aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &Opacity) == AI_SUCCESS)
		{
			materialDesc.materialData.opacity = Opacity;
		}

		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_TRANSPARENT, &Color) == AI_SUCCESS)
		{
			assert(false);
		}

		f32 metallicFactor = -1.0f;
		if (aiGetMaterialFloat(mat, AI_MATKEY_METALLIC_FACTOR, &metallicFactor) == AI_SUCCESS)
		{
			materialDesc.materialData.metalness = metallicFactor;
		}

		f32 roughnessFactor = -1.0f;
		if (aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR, &roughnessFactor) == AI_SUCCESS)
		{
			materialDesc.materialData.roughness = roughnessFactor;
		}

		aiString alphaMode;
		if (aiGetMaterialString(mat, AI_MATKEY_GLTF_ALPHAMODE, &alphaMode) == AI_SUCCESS)
		{
			if (strcmp(alphaMode.C_Str(), "OPAQUE") == 0) materialDesc.materialData.alphaMode = ALPHA_MODE_OPAQUE;
			else if (strcmp(alphaMode.C_Str(), "BLEND") == 0) materialDesc.materialData.alphaMode = ALPHA_MODE_BLEND;
			else if (strcmp(alphaMode.C_Str(), "MASK") == 0) materialDesc.materialData.alphaMode = ALPHA_MODE_MASK;
		}

		f32 alphaCutoff;
		if (aiGetMaterialFloat(mat, AI_MATKEY_GLTF_ALPHACUTOFF, &alphaCutoff) == AI_SUCCESS)
		{
			materialDesc.materialData.alphaCutoff = alphaCutoff;
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
			materialDesc.emissiveTexName = Path.C_Str();
			materialDesc.materialData.emissiveTexIdx = FindTextureId(materialDesc.emissiveTexName);
		}

		if (aiGetMaterialTexture(mat, aiTextureType_DIFFUSE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialDesc.baseColorTexName = Path.C_Str();
			materialDesc.materialData.baseColorTexIdx = FindTextureId(materialDesc.baseColorTexName);
		}

		// first try tangent space normal map
		if (aiGetMaterialTexture(mat, aiTextureType_NORMALS, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialDesc.normalTexName = Path.C_Str();
			materialDesc.materialData.normalTexIdx = FindTextureId(materialDesc.normalTexName);
		}

		if (aiGetMaterialTexture(mat, aiTextureType_LIGHTMAP, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialDesc.ambientTexName = Path.C_Str();
		}

		if (aiGetMaterialTexture(mat, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
		{
			materialDesc.roughnessMetalnessTexName = Path.C_Str();
			materialDesc.materialData.roughnessMetalnessTexIdx = FindTextureId(materialDesc.roughnessMetalnessTexName);
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
			assert(materialName == materialDesc.name);
		}

		ret.mMaterials.push_back(materialDesc);
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

