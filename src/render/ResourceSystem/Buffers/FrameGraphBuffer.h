//***************************************************************************************
// FrameGraphBuffer.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <assert.h>
#include "render/ResourceSystem/FrameGraphResource.h"
#include "render/PrimitiveTopology.h"
#include "render/VertexFormat.h"
#include "geometry/TriangleIndices.h"

namespace forward
{
	class FrameGraphBuffer : public FrameGraphResource
	{
	public:
		FrameGraphBuffer(const std::string& name);
	};

	/////////////////////////////////////////////////////////

	class FrameGraphIndexBuffer : public FrameGraphBuffer
	{
	public:
		FrameGraphIndexBuffer(const std::string& name, PrimitiveTopologyType type, u32 primitive_count);
		ResourceType GetResourceType() const override;

		PrimitiveTopologyType GetPrimitiveType() const;

		void AddFace(const TriangleIndices& face);
		//void AddLine(const LineIndices& line);
		//void AddPoint(const PointIndices& point);
		void AddIndex(u32 index);

	protected:
		PrimitiveTopologyType m_primitiveType;
	};

	class FrameGraphVertexBuffer : public FrameGraphBuffer
	{
	public:
		FrameGraphVertexBuffer(const std::string& name, const VertexFormat& vformat, u32 numVertices);
		ResourceType GetResourceType() const override;

		const VertexFormat& GetVertexFormat() const;

		template<class T>
		void AddVertex(const T& vertex);

	protected:
		VertexFormat	m_vertexFormat;
	};

	template<class T>
	void FrameGraphVertexBuffer::AddVertex(const T& vertex)
	{
		assert(m_numActiveElements < m_numElements);
		assert(sizeof(T) == m_vertexFormat.GetVertexSize());

		memcpy(m_data + m_numActiveElements, &vertex, sizeof(T));
		++m_numActiveElements;
	}


	class FrameGraphConstantBuffer : public FrameGraphBuffer
	{
	public:
		template<class T>
		FrameGraphConstantBuffer(const std::string& name, const T& data);
		ResourceType GetResourceType() const override;
		
		template<class T>
		T* GetTypedData();

		template<class T>
		void SetTypedData(const T& data);
	};


	template<class T>
	FrameGraphConstantBuffer::FrameGraphConstantBuffer(const std::string& name, const T& data)
		: FrameGraphBuffer(name)
	{
		m_type = FGOT_CONSTANT_BUFFER;
		Initialize(1, sizeof(data));
	}

	template<class T>
	T* FrameGraphConstantBuffer::GetTypedData()
	{
		return static_cast<T*>(m_data);
	}

	template<class T>
	void FrameGraphConstantBuffer::SetTypedData(const T& data)
	{
		assert(sizeof(T) == m_elementSize);

		memcpy(m_data, &data, sizeof(T));
		if (m_numActiveElements == 0)
		{
			++m_numActiveElements;
		}
	}
}