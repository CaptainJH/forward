//***************************************************************************************
// FrameGraphResource.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "FrameGraphResource.h"

using namespace forward;

FrameGraphResource::FrameGraphResource()
	: m_data(nullptr)
	, m_numBytes(0)
	, m_numElements(0)
	, m_elementSize(0)
{}

FrameGraphResource::FrameGraphResource(const std::string& name)
	: m_data(nullptr)
	, m_numBytes(0)
	, m_numElements(0)
	, m_elementSize(0)
{
	m_name = name;
}

void FrameGraphResource::SetResource(ResourcePtr resource)
{
	m_resource = resource;
}

ResourcePtr FrameGraphResource::GetResource()
{
	return m_resource;
}

const std::string& FrameGraphResource::Name() const
{
	return m_name;
}

FrameGraphResource::~FrameGraphResource()
{

}

u32 FrameGraphResource::GetNumElements() const
{
	return m_numElements;
}

u32 FrameGraphResource::GetElementSize() const
{
	return m_elementSize;
}

u32 FrameGraphResource::GetNumBytes() const
{
	return m_numBytes;
}

void FrameGraphResource::CreateStorage()
{
	if (m_storage.empty())
	{
		m_storage.resize(m_numBytes);
		if (!m_data)
		{
			m_data = m_storage.data();
		}
	}
}

void FrameGraphResource::DestroyStorage()
{
	// The intent of DestroyStorage is to free up CPU memory that is not
	// required when the resource GPU memory is all that is required.
	// The 'clear' call sets the size to 0, but the capacity remains the
	// same; that is, the memory is not freed.  The 'shrink_to_fit' call
	// is required to free the memory.
	if (!m_storage.empty() && m_data == m_storage.data())
	{
		m_data = nullptr;
		m_storage.clear();
		m_storage.shrink_to_fit();
	}
}

void FrameGraphResource::Initialize(u32 numElements, u32 elementSize)
{
	m_numElements = numElements;
	m_elementSize = elementSize;

	if (numElements > 0 && elementSize > 0)
	{
		m_numBytes = numElements * elementSize;
		CreateStorage();
	}
	else
	{
		// No assertion may occur here.  The VertexBuffer constructor with
		// a VertexFormat of zero attributes (used for SV_VertexID-based
		// drawing) and the IndexBuffer constructor for which no indices are
		// provided will lead to this path.
		m_numBytes = 0;
		m_elementSize = 0;
	}
}