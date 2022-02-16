//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
//--------------------------------------------------------------------------------
#include "DeviceResource.h"
#include "Resource.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
DeviceResource::DeviceResource(forward::GraphicsObject* obj)
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

Resource* DeviceResource::GetFrameGraphResource()
{
	return dynamic_cast<Resource*>(GraphicsObject().get());
}