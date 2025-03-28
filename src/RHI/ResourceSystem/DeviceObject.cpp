//***************************************************************************************
// DeviceObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceObject.h"
#include "GraphicsObject.h"

using namespace forward;

//--------------------------------------------------------------------------------
u32 DeviceObject::s_usDeviceObjectUID = 0;
//--------------------------------------------------------------------------------
DeviceObject::DeviceObject(forward::GraphicsObject* obj)
	: m_frameGraphObjPtr(obj)
{
	m_usInnerID = s_usDeviceObjectUID++;
	m_name = L"";
}
//--------------------------------------------------------------------------------
DeviceObject::~DeviceObject()
{}
//--------------------------------------------------------------------------------
u32 DeviceObject::GetInnerID()
{
	return m_usInnerID;
}
//--------------------------------------------------------------------------------
shared_ptr<GraphicsObject> DeviceObject::GraphicsObject()
{
	if (m_frameGraphObjPtr.expired())
	{
		return nullptr;
	}
	else
	{
		return m_frameGraphObjPtr.lock_down<forward::GraphicsObject>();
	}
}