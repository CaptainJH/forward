#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

#include "SceneData.h"
#include "FileSystem.h"
#include "Log.h"
#include "RHI/Device.h"

using namespace forward;

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
				const aiVector3D t = mesh->mTextureCoords[0][i];
				sgeo.AddVertex<Vertex_POS_UV>({ .Pos = Vector3f(v.x, v.z, v.y), 
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
		auto vf = Vertex_StandSurface::GetVertexFormat();
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
				sgeo.AddVertex<Vertex_StandSurface>({ 
					.Pos = Vector3f(v.x, v.y, v.z),
					.Normal = Vector3f(n.x, n.y, n.z), 
					.Tangent = Vector3f(t.x, t.y, t.z)
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

