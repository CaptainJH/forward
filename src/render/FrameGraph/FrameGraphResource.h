//***************************************************************************************
// FrameGraphResource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/Resource.h"

namespace forward
{
	class FrameGraphResource : public Resource
	{
	public:

		FrameGraphResource(const std::string& name);

		ResourceType	GetType() override;

		u32				GetEvictionPriority() override;
		void			SetEvictionPriority(u32 EvictionPriority) override;

		void			SetResource(ResourcePtr resouce);

	protected:
		ResourcePtr		m_resource = nullptr;
	};
}