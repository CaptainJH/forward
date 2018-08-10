#pragma once
#include "RenderPassHelper.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "utilities/Utils.h"

namespace forward
{
	struct Vertex_POS_COLOR
	{
		Vector3f Pos;
		Vector4f Color;
	};

	/// Position_Normal_Tangent_TextureUV
	struct Vertex_P_N_T_UV
	{
		Vector3f Pos;
		Vector3f Normal;
		Vector3f Tangent;
		Vector2f UV;
	};

	enum GeometryPrefab
	{
		GP_COLOR_BOX,
		GP_SCREEN_QUAD,

		GP_GRID,
		GP_SPHERE,
		GP_CYLINDER,
	};

	template<class VertexType>
	struct MeshData
	{
		std::vector<VertexType> Vertices;
		std::vector<u32> Indices32;

		std::vector<u16>& GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (auto i = 0; i < Indices32.size(); ++i)
					mIndices16[i] = static_cast<u16>(Indices32[i]);
			}

			return mIndices16;
		}

		u32 FaceCount() const
		{
			u32 result = static_cast<u32>(Indices32.size() / 3);
			assert(result * 3 == Indices32.size());
			return result;
		}

	private:
		std::vector<u16> mIndices16;
	};


	class SimpleGeometry : public IRenderPassSource
	{
	public:
		SimpleGeometry(const std::string& name, VertexFormat& format, PrimitiveTopologyType pt, const u32 vertexNum, const u32 primitiveCount);

		template<class BuilderType> 
		SimpleGeometry(const std::string& name, BuilderType builder)
			: m_VB(forward::make_shared<FrameGraphVertexBuffer>(name + "_VB", BuilderType::GetVertexFormat(), builder.GetVertexCount()))
			, m_IB(forward::make_shared<FrameGraphIndexBuffer>(name + "_IB", BuilderType::Topology, builder.GetIndexCount()))
		{
			m_VB->SetUsage(ResourceUsage::RU_IMMUTABLE);
			m_IB->SetUsage(ResourceUsage::RU_IMMUTABLE);
			builder.Initializer(this);
		}
		~SimpleGeometry();

		void AddFace(const TriangleIndices& face);
		void AddLine(const LineIndices& line);
		void AddPoint(const PointIndices& point);
		void AddIndex(u32 index);

		PrimitiveTopologyType GetPrimitiveType() const;
		//void SetPrimitiveType(PrimitiveTopologyType type);

		i32 GetPrimitiveCount() const;
		u32 GetIndexCount();

		i32 GetVertexCount();
		i32 GetVertexSize();

		template<class T>
		void AddVertex(const T& vertex)
		{
			m_VB->AddVertex(vertex);
		}

	private:
		void OnRenderPassBuilding(RenderPass&) override;

		shared_ptr<FrameGraphVertexBuffer>		m_VB;
		shared_ptr<FrameGraphIndexBuffer>		m_IB;
	};


	template<i32 Prefab>
	struct GeometryBuilder
	{};

	template<>
	struct GeometryBuilder<GeometryPrefab::GP_SCREEN_QUAD>
	{
		static const u32 VertexCount = 4;
		static const u32 IndexCount = 0;
		static const PrimitiveTopologyType Topology = PrimitiveTopologyType::PT_TRIANGLESTRIP;

		u32 GetVertexCount() const
		{
			return VertexCount;
		}

		u32 GetIndexCount() const
		{
			return IndexCount;
		}

		static forward::VertexFormat GetVertexFormat()
		{
			VertexFormat vf;
			vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

			return vf;
		}

		void Initializer(forward::SimpleGeometry* geometry)
		{
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(-1.0f, +1.0f, 0.0f), Colors::White });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(+1.0f, +1.0f, 0.0f), Colors::Black });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(-1.0f, -1.0f, 0.0f), Colors::Red });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(+1.0f, -1.0f, 0.0f), Colors::Green });
		}
	};

	template<>
	struct GeometryBuilder<GeometryPrefab::GP_COLOR_BOX>
	{
		static const u32 VertexCount = 8;
		static const u32 IndexCount = 36;
		static const PrimitiveTopologyType Topology = PrimitiveTopologyType::PT_TRIANGLELIST;

		u32 GetVertexCount() const
		{
			return VertexCount;
		}

		u32 GetIndexCount() const
		{
			return IndexCount;
		}

		static forward::VertexFormat GetVertexFormat()
		{
			VertexFormat vf;
			vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

			return vf;
		}

		void Initializer(forward::SimpleGeometry* geometry)
		{
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(-1.0f, -1.0f, -1.0f), Colors::White });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(-1.0f, +1.0f, -1.0f), Colors::Black });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(+1.0f, +1.0f, -1.0f), Colors::Red });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(+1.0f, -1.0f, -1.0f), Colors::Green });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(-1.0f, -1.0f, +1.0f), Colors::Blue });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(-1.0f, +1.0f, +1.0f), Colors::Yellow });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(+1.0f, +1.0f, +1.0f), Colors::Cyan });
			geometry->AddVertex(Vertex_POS_COLOR{ Vector3f(+1.0f, -1.0f, +1.0f), Colors::Magenta });

			geometry->AddFace(TriangleIndices(0, 1, 2));
			geometry->AddFace(TriangleIndices(0, 2, 3));

			geometry->AddFace(TriangleIndices(4, 6, 5));
			geometry->AddFace(TriangleIndices(4, 7, 6));

			geometry->AddFace(TriangleIndices(4, 5, 1));
			geometry->AddFace(TriangleIndices(4, 1, 0));

			geometry->AddFace(TriangleIndices(3, 2, 6));
			geometry->AddFace(TriangleIndices(3, 6, 7));

			geometry->AddFace(TriangleIndices(1, 5, 6));
			geometry->AddFace(TriangleIndices(1, 6, 2));

			geometry->AddFace(TriangleIndices(4, 0, 3));
			geometry->AddFace(TriangleIndices(4, 3, 7));
		}
	};

	template<>
	struct GeometryBuilder<GeometryPrefab::GP_SPHERE>
	{
		static const PrimitiveTopologyType Topology = PrimitiveTopologyType::PT_TRIANGLELIST;

		static forward::VertexFormat GetVertexFormat()
		{
			VertexFormat vf;
			vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_NORMAL, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_TANGENT, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_TEXCOORD, DataFormatType::DF_R32G32_FLOAT, 0);

			return vf;
		}

		typedef Vertex_P_N_T_UV VertexType;

		MeshData<VertexType> meshData;

		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		GeometryBuilder<GeometryPrefab::GP_SPHERE>::GeometryBuilder(f32 radius, u32 sliceCount, u32 stackCount)
		{
			//
			// Compute the vertices stating at the top pole and moving down the stacks.
			//

			// Poles: note that there will be texture coordinate distortion as there is
			// not a unique point on the texture map to assign to the pole when mapping
			// a rectangular texture onto a sphere.
			VertexType topVertex = { { 0.0f, +radius, 0.0f },{ 0.0f, +1.0f, 0.0f } ,{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } };
			VertexType bottomVertex = { { 0.0f, -radius, 0.0f },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } };

			meshData.Vertices.push_back(topVertex);

			f32 phiStep = f_PI / stackCount;
			f32 thetaStep = 2.0f*f_PI / sliceCount;

			// Compute vertices for each stack ring (do not count the poles as rings).
			for (u32 i = 1; i <= stackCount - 1; ++i)
			{
				auto phi = i * phiStep;

				// Vertices of ring.
				for (u32 j = 0; j <= sliceCount; ++j)
				{
					f32 theta = j * thetaStep;

					VertexType v;

					// spherical to cartesian
					v.Pos.x = radius * sinf(phi)*cosf(theta);
					v.Pos.y = radius * cosf(phi);
					v.Pos.z = radius * sinf(phi)*sinf(theta);

					// Partial derivative of P with respect to theta
					v.Tangent.x = -radius * sinf(phi)*sinf(theta);
					v.Tangent.y = 0.0f;
					v.Tangent.z = +radius * sinf(phi)*cosf(theta);
					v.Tangent.Normalize();

					v.Normal = v.Pos;
					v.Normal.Normalize();

					v.UV.x = theta / f_2PI;
					v.UV.y = phi / f_PI;

					meshData.Vertices.push_back(v);
				}
			}

			meshData.Vertices.push_back(bottomVertex);

			//
			// Compute indices for top stack.  The top stack was written first to the vertex buffer
			// and connects the top pole to the first ring.
			//

			for (u32 i = 1; i <= sliceCount; ++i)
			{
				meshData.Indices32.push_back(0);
				meshData.Indices32.push_back(i + 1);
				meshData.Indices32.push_back(i);
			}

			//
			// Compute indices for inner stacks (not connected to poles).
			//

			// Offset the indices to the index of the first vertex in the first ring.
			// This is just skipping the top pole vertex.
			u32 baseIndex = 1;
			u32 ringVertexCount = sliceCount + 1;
			for (u32 i = 0; i < stackCount - 2; ++i)
			{
				for (u32 j = 0; j < sliceCount; ++j)
				{
					meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j);
					meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
					meshData.Indices32.push_back(baseIndex + (i + 1)*ringVertexCount + j);

					meshData.Indices32.push_back(baseIndex + (i + 1)*ringVertexCount + j);
					meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
					meshData.Indices32.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
				}
			}

			//
			// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
			// and connects the bottom pole to the bottom ring.
			//

			// South pole vertex was added last.
			u32 southPoleIndex = (u32)meshData.Vertices.size() - 1;

			// Offset the indices to the index of the first vertex in the last ring.
			baseIndex = southPoleIndex - ringVertexCount;

			for (u32 i = 0; i < sliceCount; ++i)
			{
				meshData.Indices32.push_back(southPoleIndex);
				meshData.Indices32.push_back(baseIndex + i);
				meshData.Indices32.push_back(baseIndex + i + 1);
			}
		}

		void Initializer(forward::SimpleGeometry* geometry)
		{
			for (auto& v : meshData.Vertices)
			{
				geometry->AddVertex(v);
			}

			for (auto i = 0U; i < meshData.FaceCount(); ++i)
			{
				geometry->AddFace(TriangleIndices(
					meshData.Indices32[i * 3 + 0],
					meshData.Indices32[i * 3 + 1],
					meshData.Indices32[i * 3 + 2]));
			}

		}

		u32 GetVertexCount() const
		{
			return static_cast<u32>(meshData.Vertices.size());
		}

		u32 GetIndexCount() const
		{
			return static_cast<u32>(meshData.Indices32.size());
		}
	};
}