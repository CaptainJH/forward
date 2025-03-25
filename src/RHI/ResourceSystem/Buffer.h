//***************************************************************************************
// Buffer.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <assert.h>
#include "RHI/ResourceSystem/Resource.h"
#include "RHI/PrimitiveTopology.h"
#include "RHI/VertexFormat.h"
#include "geometry/TriangleIndices.h"
#include "geometry/LineIndices.h"
#include "geometry/PointIndices.h"

namespace forward
{
	class Buffer : public Resource
	{
	public:
		Buffer(const std::string& name);
	};

	/////////////////////////////////////////////////////////

	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer(const std::string& name, PrimitiveTopologyType type, u32 primitive_count);

		PrimitiveTopologyType GetPrimitiveType() const;

		void AddFace(const TriangleIndices& face);
		void AddLine(const LineIndices& line);
		void AddPoint(const PointIndices& point);
		void AddIndex(u32 index);

		u32 operator[](u32 index);

	protected:
		PrimitiveTopologyType m_primitiveType;
	};

	class VertexBuffer : public Buffer
	{
	public:
		VertexBuffer(const std::string& name, const VertexFormat& vformat, u32 numVertices);

		const VertexFormat& GetVertexFormat() const;

		template<class T>
		void AddVertex(const T& vertex);

	protected:
		VertexFormat	m_vertexFormat;
	};

	template<class T>
	void VertexBuffer::AddVertex(const T& vertex)
	{
		assert(m_numActiveElements < m_numElements);
		assert(sizeof(T) == m_vertexFormat.GetVertexSize());

		memcpy(m_data + m_numActiveElements * sizeof(T), &vertex, sizeof(T));
		++m_numActiveElements;
		SetDirty();
	}

	class ConstantBufferBase : public Buffer
	{
	public:
		ConstantBufferBase(const std::string& name);
		ConstantBufferBase(const std::string& name, u32 size);
	};

	template<class T>
	class ConstantBuffer : public ConstantBufferBase
	{
	public:
		ConstantBuffer(const std::string& name);

		T* GetTypedData();
		void SetTypedData(const T& data);

		ConstantBuffer& operator=(const T& rhs)
		{
			assert(GetNumBytes() == sizeof(rhs));
			memcpy(GetData(), &rhs, sizeof(rhs));
			SetDirty();
			return *this;
		}
	};


	template<class T>
	ConstantBuffer<T>::ConstantBuffer(const std::string& name)
		: ConstantBufferBase(name)
	{
		m_type = FGOT_CONSTANT_BUFFER;
		m_usage = RU_DYNAMIC_UPDATE;
		Initialize(1, sizeof(T));
	}

	template<class T>
	T* ConstantBuffer<T>::GetTypedData()
	{
		SetDirty();
		return reinterpret_cast<T*>(m_data);
	}

	template<class T>
	void ConstantBuffer<T>::SetTypedData(const T& data)
	{
		assert(sizeof(T) == m_elementSize);

		memcpy(m_data, &data, sizeof(T));
		if (m_numActiveElements == 0)
		{
			++m_numActiveElements;
		}
		SetDirty();
	}

	template<class T>
	class StructuredBuffer : public Buffer
	{
	public:
		StructuredBuffer(const std::string& name, u32 size)
			: Buffer(name)
		{
			m_type = FGOT_STRUCTURED_BUFFER;
			m_usage = RU_IMMUTABLE;
			Initialize(size, sizeof(T));
		}

		T& operator[](u32 index)
		{
			auto ret = GetData() + index * sizeof(T);
			return *(T*)ret;
		}

		const T& operator[](u32 index) const
		{
			auto ret = GetData() + index * sizeof(T);
			return *(T*)ret;
		}
	};
}