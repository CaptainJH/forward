//***************************************************************************************
// DeviceObject.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"
#include "SmartPtrs.h"

namespace forward
{
	class FrameGraphObject;

	class DeviceObject : public intrusive_ref_counter
	{
	public:
		DeviceObject(forward::FrameGraphObject* obj);
		virtual ~DeviceObject();

		const std::string&		Name() const;
		void					SetName(const std::string& name);

		u32						GetInnerID();

		shared_ptr<forward::FrameGraphObject>		FrameGraphObject();

	protected:
		std::string							m_name;
		weak_ptr<forward::FrameGraphObject> m_frameGraphObjPtr = nullptr;

		static u32						s_usDeviceObjectUID;
		u32								m_usInnerID;
	};
}