#include "Geometry.h"
#include "utilities/Log.h"
#include "math/Vector2f.h"
#include "math/Vector3f.h"

using namespace forward;

SimpleGeometry::SimpleGeometry(const std::string& name, VertexFormat& format, PrimitiveTopologyType pt, const u32 vertexNum, const u32 primitiveCount)
	: m_VB(forward::make_shared<VertexBuffer>(name + "_VB", format, vertexNum))
	, m_IB(forward::make_shared<IndexBuffer>(name + "_IB", pt, primitiveCount))
{
	m_VB->SetUsage(ResourceUsage::RU_IMMUTABLE);
	m_IB->SetUsage(ResourceUsage::RU_IMMUTABLE);
}

SimpleGeometry::~SimpleGeometry()
{
}

void SimpleGeometry::OnRenderPassBuilding(RenderPass& pass)
{
	auto& pso = pass.GetPSO<RasterPipelineStateObject>();
	if (m_IB->GetNumElements())
	{
		pass.m_ia_params.m_indexBuffer = m_IB;
	}
	pass.m_ia_params.m_topologyType = m_IB->GetPrimitiveType();

	// TODO: should automatically bind to the first available binding point, 
	// and modify VertexFormat's unit accordingly.
	pass.m_ia_params.m_vertexBuffers[0] = m_VB;
	pso.m_IAState.m_vertexLayout = m_VB->GetVertexFormat();
}

void SimpleGeometry::AddFace(const TriangleIndices& face)
{
	m_IB->AddFace(face);
}

void SimpleGeometry::AddLine(const LineIndices& line)
{
	m_IB->AddLine(line);
}

void SimpleGeometry::AddPoint(const PointIndices& point)
{
	m_IB->AddPoint(point);
}

void SimpleGeometry::AddIndex(u32 index)
{
	m_IB->AddIndex(index);
}

PrimitiveTopologyType SimpleGeometry::GetPrimitiveType() const
{
	return m_IB->GetPrimitiveType();
}

i32 SimpleGeometry::GetPrimitiveCount() const
{
	u32 count = 0;
	u32 indices = m_IB->GetNumElements();

	auto primType = GetPrimitiveType();
	switch (primType)
	{
	case PT_UNDEFINED:
		break;
	case PT_POINTLIST:
		count = indices; break;
	case PT_LINELIST:
		count = indices / 2; break;
	case PT_LINESTRIP:
		count = indices - 1; break;
	case PT_TRIANGLELIST:
		count = indices / 3; break;
	case PT_TRIANGLESTRIP:
		count = indices - 2; break;
	case PT_LINELIST_ADJ:
		count = indices / 4; break;
	case PT_LINESTRIP_ADJ:
		count = indices - 3; break;
	case PT_TRIANGLELIST_ADJ:
		count = indices / 6; break;
	case PT_TRIANGLESTRIP_ADJ:
		count = (indices - 3) / 2; break;
	case PT_1_CONTROL_POINT_PATCHLIST:
		count = indices; break;
	case PT_2_CONTROL_POINT_PATCHLIST:
		count = indices / 2; break;
	case PT_3_CONTROL_POINT_PATCHLIST:
		count = indices / 3; break;
	case PT_4_CONTROL_POINT_PATCHLIST:
		count = indices / 4; break;
	case PT_5_CONTROL_POINT_PATCHLIST:
		count = indices / 5; break;
	case PT_6_CONTROL_POINT_PATCHLIST:
		count = indices / 6; break;
	case PT_7_CONTROL_POINT_PATCHLIST:
		count = indices / 7; break;
	case PT_8_CONTROL_POINT_PATCHLIST:
		count = indices / 8; break;
	case PT_9_CONTROL_POINT_PATCHLIST:
		count = indices / 9; break;
	case PT_10_CONTROL_POINT_PATCHLIST:
		count = indices / 10; break;
	case PT_11_CONTROL_POINT_PATCHLIST:
		count = indices / 11; break;
	case PT_12_CONTROL_POINT_PATCHLIST:
		count = indices / 12; break;
	case PT_13_CONTROL_POINT_PATCHLIST:
		count = indices / 13; break;
	case PT_14_CONTROL_POINT_PATCHLIST:
		count = indices / 14; break;
	case PT_15_CONTROL_POINT_PATCHLIST:
		count = indices / 15; break;
	case PT_16_CONTROL_POINT_PATCHLIST:
		count = indices / 16; break;
	case PT_17_CONTROL_POINT_PATCHLIST:
		count = indices / 17; break;
	case PT_18_CONTROL_POINT_PATCHLIST:
		count = indices / 18; break;
	case PT_19_CONTROL_POINT_PATCHLIST:
		count = indices / 19; break;
	case PT_20_CONTROL_POINT_PATCHLIST:
		count = indices / 20; break;
	case PT_21_CONTROL_POINT_PATCHLIST:
		count = indices / 21; break;
	case PT_22_CONTROL_POINT_PATCHLIST:
		count = indices / 22; break;
	case PT_23_CONTROL_POINT_PATCHLIST:
		count = indices / 23; break;
	case PT_24_CONTROL_POINT_PATCHLIST:
		count = indices / 24; break;
	case PT_25_CONTROL_POINT_PATCHLIST:
		count = indices / 25; break;
	case PT_26_CONTROL_POINT_PATCHLIST:
		count = indices / 26; break;
	case PT_27_CONTROL_POINT_PATCHLIST:
		count = indices / 27; break;
	case PT_28_CONTROL_POINT_PATCHLIST:
		count = indices / 28; break;
	case PT_29_CONTROL_POINT_PATCHLIST:
		count = indices / 29; break;
	case PT_30_CONTROL_POINT_PATCHLIST:
		count = indices / 30; break;
	case PT_31_CONTROL_POINT_PATCHLIST:
		count = indices / 31; break;
	case PT_32_CONTROL_POINT_PATCHLIST:
		count = indices / 32; break;
	}

	return count;
}

u32 SimpleGeometry::GetIndexCount()
{
	return m_IB->GetNumElements();
}

i32 SimpleGeometry::GetVertexCount()
{
	return m_VB->GetNumElements();
}

i32 SimpleGeometry::GetVertexSize()
{
	return m_VB->GetElementSize();
}
