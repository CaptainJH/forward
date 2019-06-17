//***************************************************************************************
// FrameGraphResource.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "FrameGraphResource.h"

using namespace forward;

FrameGraphResource::FrameGraphResource()
	: m_numElements(0)
	, m_numActiveElements(0)
    , m_elementSize(0)
    , m_numBytes(0)
    , m_data(nullptr)
	, m_usage(ResourceUsage::RU_IMMUTABLE)
{}

FrameGraphResource::FrameGraphResource(const std::string& name)
	: m_numElements(0)
    , m_numActiveElements(0)
	, m_elementSize(0)
    , m_numBytes(0)
    , m_data(nullptr)
	, m_usage(ResourceUsage::RU_IMMUTABLE)
{
	m_name = name;
}

DeviceResource* FrameGraphResource::GetResource()
{
	return static_cast<DeviceResource*>(m_deviceObjectPtr.get());
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

u8* FrameGraphResource::GetData()
{
	return m_data;
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

void FrameGraphResource::SetUsage(ResourceUsage usage)
{
	m_usage = usage;
}

ResourceUsage FrameGraphResource::GetUsage() const
{
	return m_usage;
}

void FrameGraphResource::CopyPitched2(u32 numRows, u32 srcRowPitch, const u8* srcData, u32 dstRowPitch, u8* dstData)
{
	if (srcRowPitch == dstRowPitch)
	{
		// The memory is contiguous.
		memcpy(dstData, srcData, dstRowPitch * numRows);
	}
	else
	{
		// Padding was added to each row of the texture, so we must
		// copy a row at a time to compensate for differing pitches.
		auto numRowBytes = std::min(srcRowPitch, dstRowPitch);
		auto srcRow = srcData;
		auto dstRow = dstData;
		for (auto row = 0U; row < numRows; ++row)
		{
			memcpy(dstRow, srcRow, numRowBytes);
			srcRow += srcRowPitch;
			dstRow += dstRowPitch;
		}
	}
}


void FrameGraphResource::CopyPitched3(u32 numRows, u32 numSlices, u32 srcRowPitch, u32 srcSlicePitch, const u8* srcData,
	u32 dstRowPitch, u32 dstSlicePitch, u8* dstData)
{
	if (srcRowPitch == dstRowPitch && srcSlicePitch == dstSlicePitch)
	{
		// The memory is contiguous.
		memcpy(dstData, srcData, dstSlicePitch * numSlices);
	}
	else
	{
		// Padding was added to each row and/or slice of the texture, so
		// we must copy the data to compensate for differing pitches.
		auto numRowBytes = std::min(srcRowPitch, dstRowPitch);
		auto srcSlice = srcData;
		auto dstSlice = dstData;
		for (auto slice = 0U; slice < numSlices; ++slice)
		{
			auto srcRow = srcSlice;
			auto dstRow = dstSlice;
			for (auto row = 0U; row < numRows; ++row)
			{
				memcpy(dstRow, srcRow, numRowBytes);
				srcRow += srcRowPitch;
				dstRow += dstRowPitch;
			}
			srcSlice += srcSlicePitch;
			dstSlice += dstSlicePitch;
		}
	}
}
