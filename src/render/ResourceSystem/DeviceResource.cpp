//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
//--------------------------------------------------------------------------------
#include "DeviceResource.h"
#include "FrameGraphResource.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
DeviceResource::DeviceResource(forward::FrameGraphObject* obj)
	: DeviceObject(obj)
{
	m_usInnerID = s_usResourceUID++;
}

DeviceResource::DeviceResource( )
	: DeviceObject(nullptr)
{
	m_usInnerID = s_usResourceUID;
	s_usResourceUID++;
}
//--------------------------------------------------------------------------------
DeviceResource::~DeviceResource()
{
}

FrameGraphResource* DeviceResource::GetFrameGraphResource()
{
	return dynamic_cast<FrameGraphResource*>(FrameGraphObject());
}