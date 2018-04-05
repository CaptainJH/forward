#pragma once
#include "RenderPassHelper.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"
#include "Vector3f.h"
#include "utilities/Utils.h"

namespace forward
{
	struct Vertex_POS_COLOR
	{
		Vector3f Pos;
		Vector4f Color;
	};

	enum GeometryPrefab
	{
		GP_COLOR_BOX,
		GP_SCREEN_QUAD,
	};


	class SimpleGeometry : public IRenderPassSource
	{
	public:
		SimpleGeometry(const std::string& name, VertexFormat& format, PrimitiveTopologyType pt, const u32 vertexNum, const u32 primitiveCount);

		template<class BuilderType> 
		SimpleGeometry(const std::string& name, BuilderType)
			: m_VB(forward::make_shared<FrameGraphVertexBuffer>(name + "_VB", BuilderType::GetVertexFormat(), BuilderType::VertexCount))
			, m_IB(forward::make_shared<FrameGraphIndexBuffer>(name + "_IB", BuilderType::Topology, BuilderType::IndexCount))
		{
			m_VB->SetUsage(ResourceUsage::RU_IMMUTABLE);
			m_IB->SetUsage(ResourceUsage::RU_IMMUTABLE);
			BuilderType::Initializer(this);
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

		static forward::VertexFormat GetVertexFormat()
		{
			VertexFormat vf;
			vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

			return vf;
		}

		static void Initializer(forward::SimpleGeometry* geometry)
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

		static forward::VertexFormat GetVertexFormat()
		{
			VertexFormat vf;
			vf.Bind(VASemantic::VA_POSITION, DataFormatType::DF_R32G32B32_FLOAT, 0);
			vf.Bind(VASemantic::VA_COLOR, DataFormatType::DF_R32G32B32A32_FLOAT, 0);

			return vf;
		}

		static void Initializer(forward::SimpleGeometry* geometry)
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
}