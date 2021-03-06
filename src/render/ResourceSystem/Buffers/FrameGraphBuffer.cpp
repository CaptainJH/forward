//***************************************************************************************
// FrameGraphBuffer.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphBuffer.h"

using namespace forward;

FrameGraphBuffer::FrameGraphBuffer(const std::string& name)
	: FrameGraphResource(name)
{
	m_type = FGOT_BUFFER;
}

FrameGraphIndexBuffer::FrameGraphIndexBuffer(const std::string& name, PrimitiveTopologyType type, u32 primitive_count)
	: FrameGraphBuffer(name)
	, m_primitiveType(type)
{
	m_type = FGOT_INDEX_BUFFER;
	Initialize(primitive_count, sizeof(u32));
}

PrimitiveTopologyType FrameGraphIndexBuffer::GetPrimitiveType() const
{
	return m_primitiveType;
}

void FrameGraphIndexBuffer::AddFace(const TriangleIndices& face)
{
	assert(m_numActiveElements < m_numElements);

	u32 src = face.P1();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;

	assert(m_numActiveElements < m_numElements);

	src = face.P2();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;

	assert(m_numActiveElements < m_numElements);

	src = face.P3();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;
}

void FrameGraphIndexBuffer::AddLine(const LineIndices& line)
{
	assert(m_numActiveElements < m_numElements);

	u32 src = line.P1();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;

	assert(m_numActiveElements < m_numElements);

	src = line.P2();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;
}

void FrameGraphIndexBuffer::AddPoint(const PointIndices& point)
{
	assert(m_numActiveElements < m_numElements);

	u32 src = point.P1();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;
}

void FrameGraphIndexBuffer::AddIndex(u32 index)
{
	assert(m_numActiveElements < m_numElements);

	memcpy(m_data + m_numActiveElements * sizeof(u32), &index, sizeof(u32));
	++m_numActiveElements;
}

u32 FrameGraphIndexBuffer::operator[](u32 index)
{
	assert(index < m_numActiveElements);
	return *(u32*)GetData()[index * sizeof(u32)];
}

FrameGraphVertexBuffer::FrameGraphVertexBuffer(const std::string& name, const VertexFormat& vformat, u32 numVertices)
	: FrameGraphBuffer(name)
	, m_vertexFormat(vformat)
{
	m_type = FGOT_VERTEX_BUFFER;
	Initialize(numVertices, m_vertexFormat.GetVertexSize());
}

const VertexFormat& FrameGraphVertexBuffer::GetVertexFormat() const
{
	return m_vertexFormat;
}

FrameGraphConstantBufferBase::FrameGraphConstantBufferBase(const std::string& name)
	: FrameGraphBuffer(name)
{
	m_usage = RU_DYNAMIC_UPDATE;
}