//***************************************************************************************
// FrameGraphBuffer.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphBuffer.h"

using namespace forward;

FrameGraphBuffer::FrameGraphBuffer(const std::string& name)
	: FrameGraphResource(name)
{}

FrameGraphIndexBuffer::FrameGraphIndexBuffer(const std::string& name, PrimitiveTopologyType type, u32 primitive_count)
	: FrameGraphBuffer(name)
	, m_primitiveType(type)
{
	Initialize(primitive_count, sizeof(u32));
}

ResourceType FrameGraphIndexBuffer::GetResourceType() const
{
	return RT_INDEXBUFFER;
}

PrimitiveTopologyType FrameGraphIndexBuffer::GetPrimitiveType() const
{
	return m_primitiveType;
}

FrameGraphVertexBuffer::FrameGraphVertexBuffer(const std::string& name)
	: FrameGraphBuffer(name)
{}

ResourceType FrameGraphVertexBuffer::GetResourceType() const
{
	return RT_VERTEXBUFFER;
}

FrameGraphConstantBuffer::FrameGraphConstantBuffer(const std::string& name)
	: FrameGraphBuffer(name)
{}

ResourceType FrameGraphConstantBuffer::GetResourceType() const
{
	return RT_CONSTANTBUFFER;
}