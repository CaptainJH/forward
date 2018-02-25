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

		virtual ~DeviceObject();

		const std::string&		Name() const;
		void					SetName(const std::string& name);

	protected:
		std::string				m_name;
	};
}