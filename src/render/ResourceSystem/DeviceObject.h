//***************************************************************************************
// DeviceObject.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"

namespace forward
{
	class DeviceObject
	{
	public:
		DeviceObject();
		virtual ~DeviceObject();

		const std::string&		Name() const;
		void					SetName(const std::string& name);

		u32						GetInnerID();

	protected:
		std::string				m_name;

		static u32				s_usResourceUID;
		u32						m_usInnerID;
	};
}