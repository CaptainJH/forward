//***************************************************************************************
// FrameGraphBuffer.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphBuffer.h"

using namespace forward;

FrameGraphBuffer::FrameGraphBuffer(const std::string& name)
	: FrameGraphResource(name)
{}

FrameGraphIndexBuffer::FrameGraphIndexBuffer(const std::string& name)
	: FrameGraphBuffer(name)
{}

ResourceType FrameGraphIndexBuffer::GetResourceType() const
{
	return RT_INDEXBUFFER;
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