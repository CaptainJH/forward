//***************************************************************************************
// ResourceProxy.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "ResourceProxy.h"

using namespace forward;

ResourceProxy::ResourceProxy()
	: m_iResource(-1)
{
	m_iResourceSRV = m_iResourceRTV = m_iResourceDSV = m_iResourceUAV = 0;
}

ResourceProxy::~ResourceProxy()
{

}

