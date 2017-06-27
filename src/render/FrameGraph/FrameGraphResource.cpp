//***************************************************************************************
// FrameGraphResource.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "FrameGraphResource.h"

using namespace forward;

ResourceType FrameGraphResource::GetType()
{
	return m_resource->GetType();
}

u32 FrameGraphResource::GetEvictionPriority()
{
	return m_resource->GetEvictionPriority();
}

void FrameGraphResource::SetEvictionPriority(u32 EvictionPriority)
{
	m_resource->SetEvictionPriority(EvictionPriority);
}

FrameGraphResource::FrameGraphResource(const std::string& name)
{
	m_name = name;
}

void FrameGraphResource::SetResource(ResourcePtr resource)
{
	m_resource = resource;
}