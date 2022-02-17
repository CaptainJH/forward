#include "SceneData.h"
#include "FileSystem.h"
#include "Log.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

using namespace forward;

SceneData SceneData::LoadFromFile(const std::wstring fileName)
{
	SceneData ret;

	const std::wstring sceneFilePathW = FileSystem::getSingleton().GetModelsFolder();
	const std::string sceneFileFullPath = TextHelper::ToAscii(sceneFilePathW + fileName);

	const aiScene* scene = aiImportFile(sceneFileFullPath.c_str(), aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_FlipWindingOrder);

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
		SimpleGeometry sgeo(mesh->mName.C_Str(), vf, PrimitiveTopologyType::PT_TRIANGLELIST, 
			mesh->mNumVertices, mesh->mNumFaces * 3);

		for (auto i = 0U; i != mesh->mNumVertices; i++)
		{
			const aiVector3D v = mesh->mVertices[i];
			const aiVector3D t = mesh->mTextureCoords[0][i];
			sgeo.AddVertex<Vertex_POS_UV>({ .Pos = Vector3f(v.x, v.z, v.y), .UV = Vector2f(t.x, t.y) });
		}

		for (auto i = 0U; i != mesh->mNumFaces; i++)
		{
			assert(mesh->mFaces[i].mNumIndices == 3);
			sgeo.AddFace(TriangleIndices(mesh->mFaces[i].mIndices[0],
				mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]));
		}

		ret.mMeshData.push_back(sgeo);
	}

	aiReleaseImport(scene);
	return ret;
}

