//***************************************************************************************
// FrameGraphResource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/DeviceResource.h"

namespace forward
{
	class FrameGraphResource
	{
	public:
		FrameGraphResource();
		FrameGraphResource(const std::string& name);
		virtual ~FrameGraphResource();

		virtual ResourceType GetResourceType() const = 0;

		void			SetResource(ResourcePtr resouce);
		ResourcePtr		GetResource();

		const std::string& Name() const;

		// Create or destroy the system-memory storage associated with the
		// resource.  A resource does not necessarily require system memory
		// if it is intended only to provide information for GPU-resource
		// creation.
		void CreateStorage();
		void DestroyStorage();

		u32 GetNumElements() const;
		u32 GetElementSize() const;
		u32 GetNumBytes() const;

		void Initialize(u32 numElements, u32 elementSize);

	protected:
		ResourcePtr		m_resource = nullptr;
		std::string		m_name;

		u32		m_numElements;
		u32		m_elementSize;
		u32		m_numBytes;
		std::vector<i8> m_storage;
		i8*		m_data;
	};
}