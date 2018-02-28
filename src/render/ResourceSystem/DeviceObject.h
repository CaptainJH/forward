//***************************************************************************************
// DeviceObject.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"

namespace forward
{
	class FrameGraphObject;

	class DeviceObject
	{
	public:
		DeviceObject(forward::FrameGraphObject* obj);
		virtual ~DeviceObject();

		const std::string&		Name() const;
		void					SetName(const std::string& name);

		u32						GetInnerID();

		forward::FrameGraphObject*		FrameGraphObject();

	protected:
		std::string				m_name;
		forward::FrameGraphObject*		m_frameGraphObjPtr = nullptr;

		static u32				s_usResourceUID;
		u32						m_usInnerID;
	};
}