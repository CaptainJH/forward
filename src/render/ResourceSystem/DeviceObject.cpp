//***************************************************************************************
// DeviceObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceObject.h"

using namespace forward;

//--------------------------------------------------------------------------------
u32 DeviceObject::s_usResourceUID = 0;
//--------------------------------------------------------------------------------
DeviceObject::DeviceObject(forward::FrameGraphObject* obj)
	: m_frameGraphObjPtr(obj)
{
	m_usInnerID = s_usResourceUID;
	s_usResourceUID++;
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
FrameGraphObject* DeviceObject::FrameGraphObject()
{
	return m_frameGraphObjPtr;
}