#include "Log.h"
#include "GeometryLoader.h"
#include "FileSystem.h"
#include "BufferConfigDX11.h"
#include "TriangleIndices.h"
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include <sstream>

using namespace forward;

i32 GeometryLoader::loadMeshFileDX(std::wstring filename, std::vector<GeometryPtr>& outMeshes, std::vector<Matrix4f>& outMtx)
{
	// load mesh file
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;

	const std::wstring meshFileNameW = filename;
	const std::wstring meshFilePathW = FileSystem::getSingleton().GetModelsFolder();
	const std::string meshFileFullPath = TextHelper::ToAscii(meshFilePathW + meshFileNameW);

	pScene = imp.ReadFile(meshFileFullPath, aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_FlipWindingOrder);
	if (!pScene)
	{
		std::wstringstream wss;
		wss << meshFileNameW << L" loading failed!";
		std::wstring errText = wss.str();
		Log::Get().Write(errText);
		return -1;
	}

	auto count = pScene->mNumMeshes;
	if (count < 1)
	{
		std::wstringstream wss;
		wss << meshFileNameW << L" ONLY contains " << count << L" mesh !";
		std::wstring errText = wss.str();
		Log::Get().Write(errText);
		return count;
	}

	// load a scene into GeometryDX11
	for (auto id = 0U; id < count; ++id)
	{
		auto pMesh = pScene->mMeshes[id];
		auto pGeometry = GeometryPtr(new GeometryDX11());

		const i32 NumVertex = pMesh->mNumVertices;

		if (pMesh->HasPositions())
		{
			// create the vertex element streams
			VertexElementDX11* pPositions = new VertexElementDX11(3, NumVertex);
			pPositions->m_SemanticName = VertexElementDX11::PositionSemantic;
			pPositions->m_uiSemanticIndex = 0;
			pPositions->m_Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pPositions->m_uiInputSlot = 0;
			pPositions->m_uiAlignedByteOffset = 0;
			pPositions->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pPositions->m_uiInstanceDataStepRate = 0;

			pGeometry->AddElement(pPositions);

			for (auto i = 0; i < NumVertex; ++i)
			{
				aiVector3D pos = pMesh->mVertices[i];
				*pPositions->Get3f(i) = Vector3f(pos.x, pos.y, pos.z);
			}
		}

		if (pMesh->HasNormals())
		{
			VertexElementDX11* pNormals = new VertexElementDX11(3, NumVertex);
			pNormals->m_SemanticName = VertexElementDX11::NormalSemantic;
			pNormals->m_uiSemanticIndex = 0;
			pNormals->m_Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pNormals->m_uiInputSlot = 0;
			pNormals->m_uiAlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pNormals->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pNormals->m_uiInstanceDataStepRate = 0;

			pGeometry->AddElement(pNormals);

			for (auto i = 0; i < NumVertex; ++i)
			{
				aiVector3D normal = pMesh->mNormals[i];
				*pNormals->Get3f(i) = Vector3f(normal.x, normal.y, normal.z);
			}
		}

		//if (pMesh->HasVertexColors(0))
		{
			VertexElementDX11* pColors = new VertexElementDX11(4, NumVertex);
			pColors->m_SemanticName = VertexElementDX11::ColorSemantic;
			pColors->m_uiSemanticIndex = 0;
			pColors->m_Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pColors->m_uiInputSlot = 0;
			pColors->m_uiAlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pColors->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pColors->m_uiInstanceDataStepRate = 0;

			pGeometry->AddElement(pColors);

			for (auto i = 0; i < NumVertex; ++i)
			{
				Vector4f color = Colors::Green;
				if (pMesh->HasVertexColors(0))
				{
					auto c = pMesh->mColors[0][i];
					color = Vector4f(c.r, c.g, c.b, c.a);
				}
				*pColors->Get4f(i) = color;
			}
		}

		for (auto i = 0U; i < pMesh->mNumFaces; ++i)
		{
			aiFace face = pMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			pGeometry->AddFace(TriangleIndices(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
		}

		outMeshes.push_back(pGeometry);
	}

	count = pScene->mRootNode->mNumChildren;
	for (auto i = 0U; i < count; ++i)
	{
		Matrix4f mat(true);
		aiNode* node = pScene->mRootNode->mChildren[i];
		for (auto j = 0; j < 4; ++j)
		{
			aiMatrix4x4 trans = node->mTransformation;
			auto ptr = trans[j];
			mat(j, 0) = ptr[0];
			mat(j, 1) = ptr[1];
			mat(j, 2) = ptr[2];
			mat(j, 3) = ptr[3];
		}

		mat = mat.Transpose();
		outMtx.push_back(mat);
	}

	return count;
}