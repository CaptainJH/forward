//***************************************************************************************
// FrameGraphBuffer.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once


#include "render/ResourceSystem/FrameGraphResource.h"

namespace forward
{
	class FrameGraphBuffer : public FrameGraphResource
	{
	public:
		FrameGraphBuffer(const std::string& name);
	};

	class FrameGraphIndexBuffer : public FrameGraphBuffer
	{
	public:
		FrameGraphIndexBuffer(const std::string& name);
		ResourceType GetResourceType() const override;
	};

	class FrameGraphVertexBuffer : public FrameGraphBuffer
	{
	public:
		FrameGraphVertexBuffer(const std::string& name);
		ResourceType GetResourceType() const override;
	};

	class FrameGraphConstantBuffer : public FrameGraphBuffer
	{
	public:
		FrameGraphConstantBuffer(const std::string& name);
		ResourceType GetResourceType() const override;
	};
}