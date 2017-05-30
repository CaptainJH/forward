//***************************************************************************************
// ResourceProxy.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"

namespace forward
{
	class Renderer;
	class Texture2dConfig;

	class ResourceProxy
	{
	public:
		ResourceProxy();

		virtual ~ResourceProxy();

	public:

		// internal use
		i32						m_iResourceSRV;
		i32						m_iResourceRTV;
		i32						m_iResourceDSV;
		i32						m_iResourceUAV;


	public:
		i32	m_iResource;
	};

	typedef std::shared_ptr<ResourceProxy> ResourcePtrBase;
};