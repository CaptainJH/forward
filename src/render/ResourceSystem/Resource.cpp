//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
//--------------------------------------------------------------------------------
#include "Resource.h"
#include "ResourceConfig.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
u32 Resource::s_usResourceUID = 0;
//--------------------------------------------------------------------------------
Resource::Resource( )
{
	m_name = "";
	m_usInnerID = s_usResourceUID;
	s_usResourceUID++;
}
//--------------------------------------------------------------------------------
Resource::Resource(const std::string& name)
	: m_name(name)
{
	m_usInnerID = s_usResourceUID;
	s_usResourceUID++;
}
//--------------------------------------------------------------------------------
Resource::~Resource()
{
	SAFE_DELETE(m_pResourceConfig);
}
//--------------------------------------------------------------------------------
u32 Resource::GetInnerID()
{
	return( m_usInnerID );
}
//--------------------------------------------------------------------------------
const std::string& Resource::Name() const
{
	return m_name;
}
//--------------------------------------------------------------------------------
void Resource::SetName(const std::string& name)
{
	m_name = name;
}
//--------------------------------------------------------------------------------
void Resource::SetResourceConfig(ResourceConfig* pConfig) 
{ 
	m_pResourceConfig = pConfig; 
}