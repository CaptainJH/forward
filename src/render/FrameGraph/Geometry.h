#pragma once
#include "RenderPassHelper.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"

namespace forward
{
	class SimpleGeometry : public IRenderPassSource
	{
	public:
		SimpleGeometry(const std::string& name, VertexFormat& format, PrimitiveTopologyType pt, const u32 vertexNum, const u32 primitiveCount);
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

	private:
		void OnRenderPassBuilding(RenderPass&) override;

		FrameGraphVertexBuffer		m_VB;
		FrameGraphIndexBuffer		m_IB;
	};
}