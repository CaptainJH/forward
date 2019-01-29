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
		virtual void			PostSetDeviceObject(forward::FrameGraphObject*) {}

	protected:
		std::string									m_name;
		weak_ptr<forward::intrusive_ref_counter>	m_frameGraphObjPtr = nullptr; // use type intrusive_ref_counter to avoid cross reference compiling issue.

		static u32						s_usDeviceObjectUID;
		u32								m_usInnerID;
	};

	typedef shared_ptr<DeviceObject> DeviceObjPtr;
}