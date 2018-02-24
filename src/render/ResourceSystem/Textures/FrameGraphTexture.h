//***************************************************************************************
// FrameGraphTexture.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/FrameGraphResource.h"

namespace forward
{
	class FrameGraphTexture : public FrameGraphResource
	{
	public:
		FrameGraphTexture(const std::string& name);
	};

	class FrameGraphTexture2D : public FrameGraphTexture
	{
	public:
		FrameGraphTexture2D(const std::string& name);
		ResourceType GetResourceType() const override;
	};
}