//***************************************************************************************
// DeviceObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceObject.h"
#include "FrameGraph/FrameGraphObject.h"

using namespace forward;

//--------------------------------------------------------------------------------
u32 DeviceObject::s_usDeviceObjectUID = 0;
//--------------------------------------------------------------------------------
DeviceObject::DeviceObject(forward::FrameGraphObject* obj)
	: m_frameGraphObjPtr(obj)
{
	m_usInnerID = s_usDeviceObjectUID++;
}
//--------------------------------------------------------------------------------
DeviceObject::~DeviceObject()
{
}
//--------------------------------------------------------------------------------
const std::string& DeviceObject::Name() const
{
	return m_name;
}
//--------------------------------------------------------------------------------
void DeviceObject::SetName(const std::string& name)
{
	m_name = name;
}
//--------------------------------------------------------------------------------
u32 DeviceObject::GetInnerID()
{
	return m_usInnerID;
}
//--------------------------------------------------------------------------------
shared_ptr<FrameGraphObject> DeviceObject::FrameGraphObject()
{
	if (m_frameGraphObjPtr.expired())
	{
		return nullptr;
	}
	else
	{
		return m_frameGraphObjPtr.lock_down<forward::FrameGraphObject>();
	}
}