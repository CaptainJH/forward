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
}

DeviceResource::DeviceResource( )
	: DeviceObject(nullptr)
{
}
//--------------------------------------------------------------------------------
DeviceResource::~DeviceResource()
{
}

FrameGraphResource* DeviceResource::GetFrameGraphResource()
{
	return dynamic_cast<FrameGraphResource*>(FrameGraphObject().get());
}