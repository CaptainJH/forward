//***************************************************************************************
// FrameGraphObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "PCH.h"
#include "FrameGraphObject.h"
#include "Log.h"
#include "render/ResourceSystem/DeviceObject.h"

using namespace forward;

std::vector<weak_ptr<FrameGraphObject>> FrameGraphObject::m_sFGObjs;

FrameGraphObject::FrameGraphObject()
{
	RegisterObject(this);
}

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

shared_ptr<FrameGraphObject> FrameGraphObject::FindFrameGraphObject(const std::string& name)
{
	for (auto ptr : m_sFGObjs)
	{
		if (!ptr.expired())
		{
			auto shared = ptr.lock();
			if (shared->Name() == name)
			{
				return shared;
			}
		}
	}

	return nullptr;
}

void FrameGraphObject::RegisterObject(FrameGraphObject* ptr)
{
	auto it = std::find_if(m_sFGObjs.begin(), m_sFGObjs.end(), [](weak_ptr<FrameGraphObject>& ptr)->bool {
		return ptr.expired();
	});
	if (it == m_sFGObjs.end())
	{
		m_sFGObjs.push_back(ptr);
	}
	else
	{
		it->reset(ptr);
	}
}

void FrameGraphObject::CheckMemoryLeak()
{
	for (auto ptr : m_sFGObjs)
	{
		if (!ptr.expired())
		{
			std::wstringstream wss;
			wss << L"Memory Leak: " << ptr.lock().get() << std::endl;
			std::wstring text = wss.str();
			Log::Get().Write(text);
		}
	}
}

void FrameGraphObject::SetDeviceObject(forward::DeviceObject* obj)
{
	m_deviceObjectPtr = obj;
	PostSetDeviceObject();
}

void FrameGraphObject::SetDeviceObject(forward::shared_ptr<forward::DeviceObject> p)
{
	m_deviceObjectPtr = p;
	PostSetDeviceObject();
}