//***************************************************************************************
// Buffer.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "Buffer.h"

using namespace forward;

Buffer::Buffer(const std::string& name)
	: Resource(name)
{
	m_type = FGOT_BUFFER;
}

IndexBuffer::IndexBuffer(const std::string& name, PrimitiveTopologyType type, u32 primitive_count)
	: Buffer(name)
	, m_primitiveType(type)
{
	m_type = FGOT_INDEX_BUFFER;
	Initialize(primitive_count, sizeof(u32));
}

PrimitiveTopologyType IndexBuffer::GetPrimitiveType() const
{
	return m_primitiveType;
}

void IndexBuffer::AddFace(const TriangleIndices& face)
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

void IndexBuffer::AddLine(const LineIndices& line)
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

void IndexBuffer::AddPoint(const PointIndices& point)
{
	assert(m_numActiveElements < m_numElements);

	u32 src = point.P1();
	memcpy(m_data + m_numActiveElements * sizeof(u32), &src, sizeof(u32));
	++m_numActiveElements;
}

void IndexBuffer::AddIndex(u32 index)
{
	assert(m_numActiveElements < m_numElements);

	memcpy(m_data + m_numActiveElements * sizeof(u32), &index, sizeof(u32));
	++m_numActiveElements;
}

u32 IndexBuffer::operator[](u32 index)
{
	assert(index < m_numActiveElements);
	return *(u32*)GetData()[index * sizeof(u32)];
}

VertexBuffer::VertexBuffer(const std::string& name, const VertexFormat& vformat, u32 numVertices)
	: Buffer(name)
	, m_vertexFormat(vformat)
{
	m_type = FGOT_VERTEX_BUFFER;
	Initialize(numVertices, m_vertexFormat.GetVertexSize());
}

const VertexFormat& VertexBuffer::GetVertexFormat() const
{
	return m_vertexFormat;
}

ConstantBufferBase::ConstantBufferBase(const std::string& name)
	: Buffer(name)
{
	m_usage = RU_DYNAMIC_UPDATE;
}