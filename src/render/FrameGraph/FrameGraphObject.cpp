//***************************************************************************************
// FrameGraphObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphObject.h"

using namespace forward;

FrameGraphObject::~FrameGraphObject()
{
}

const std::string& FrameGraphObject::Name() const
{
	return m_name;
}

void FrameGraphObject::SetName(const std::string& name)
{
	m_name = name;
}