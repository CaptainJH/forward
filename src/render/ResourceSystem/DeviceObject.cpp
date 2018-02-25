//***************************************************************************************
// DeviceObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceObject.h"

using namespace forward;

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