//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
//--------------------------------------------------------------------------------
#include "DeviceResource.h"
#include "ResourceConfig.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
u32 DeviceResource::s_usResourceUID = 0;
//--------------------------------------------------------------------------------
DeviceResource::DeviceResource( )
{
	m_name = "";
	m_usInnerID = s_usResourceUID;
	s_usResourceUID++;
}
//--------------------------------------------------------------------------------
DeviceResource::DeviceResource(const std::string& name)
	: m_name(name)
{
	m_usInnerID = s_usResourceUID;
	s_usResourceUID++;
}
//--------------------------------------------------------------------------------
DeviceResource::~DeviceResource()
{
}
//--------------------------------------------------------------------------------
u32 DeviceResource::GetInnerID()
{
	return( m_usInnerID );
}
//--------------------------------------------------------------------------------
const std::string& DeviceResource::Name() const
{
	return m_name;
}
//--------------------------------------------------------------------------------
void DeviceResource::SetName(const std::string& name)
{
	m_name = name;
}