//***************************************************************************************
// FrameGraphObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphObject.h"

using namespace forward;

std::vector<FrameGraphObject*> FrameGraphObject::m_sFGObjs;

FrameGraphObject::FrameGraphObject()
{
	RegisterObject(this);
}

FrameGraphObject::~FrameGraphObject()
{
	auto it = std::find(m_sFGObjs.begin(), m_sFGObjs.end(), this);
	if (it != m_sFGObjs.end())
	{
		*it = nullptr;
	}
}

const std::string& FrameGraphObject::Name() const
{
	return m_name;
}

void FrameGraphObject::SetName(const std::string& name)
{
	m_name = name;
}

FrameGraphObject* FrameGraphObject::FindFrameGraphObject(const std::string& name)
{
	for (auto ptr : m_sFGObjs)
	{
		if (ptr->Name() == name)
		{
			return ptr;
		}
	}

	return nullptr;
}

void FrameGraphObject::RegisterObject(FrameGraphObject* ptr)
{
	auto it = std::find(m_sFGObjs.begin(), m_sFGObjs.end(), nullptr);
	if (it == m_sFGObjs.end())
	{
		m_sFGObjs.push_back(ptr);
	}
	else
	{
		*it = ptr;
	}
}