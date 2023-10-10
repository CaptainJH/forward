//***************************************************************************************
// GraphicsObject.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "PCH.h"
#include "GraphicsObject.h"
#include "Log.h"
#include "RHI/ResourceSystem/DeviceObject.h"

using namespace forward;

std::vector<weak_ptr<GraphicsObject>> GraphicsObject::m_sFGObjs;

GraphicsObject::GraphicsObject()
{
	RegisterObject(this);
}

GraphicsObject::~GraphicsObject()
{
}

const std::string& GraphicsObject::Name() const
{
	return m_name;
}

void GraphicsObject::SetName(const std::string& name)
{
	m_name = name;
}

void GraphicsObject::RegisterObject(GraphicsObject* ptr)
{
	auto it = std::find_if(m_sFGObjs.begin(), m_sFGObjs.end(), [](weak_ptr<GraphicsObject>& ptr)->bool {
		return ptr.expired();
	});
	if (it == m_sFGObjs.end())
	{
		m_sFGObjs.emplace_back(ptr);
	}
	else
	{
		it->reset(ptr);
	}
}

void GraphicsObject::CheckMemoryLeak()
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

void GraphicsObject::SetDeviceObject(forward::DeviceObject* obj)
{
	m_deviceObjectPtr = obj;
	if (obj)obj->PostSetDeviceObject(this);
}

void GraphicsObject::SetDeviceObject(DeviceObjPtr p)
{
	m_deviceObjectPtr = p;
	p->PostSetDeviceObject(this);
}