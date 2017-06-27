//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"
//--------------------------------------------------------------------------------
namespace forward
{
	enum ResourceType
	{
		RT_VERTEXBUFFER = 0x010000,
		RT_INDEXBUFFER = 0x020000,
		RT_CONSTANTBUFFER = 0x030000,
		RT_STRUCTUREDBUFFER = 0x040000,
		RT_BYTEADDRESSBUFFER = 0x050000,
		RT_INDIRECTARGSBUFFER = 0x060000,
		RT_TEXTURE1D = 0x070000,
		RT_TEXTURE2D = 0x080000,
		RT_TEXTURE3D = 0x090000
	};

	class ResourceConfig;

	class Resource
	{
	public:
		Resource();
		Resource(const std::string& name);

		virtual ~Resource();

		virtual ResourceType	GetType() = 0;

		virtual u32				GetEvictionPriority() = 0;
		virtual void			SetEvictionPriority( u32 EvictionPriority ) = 0;

		u32						GetInnerID();

		const std::string&		Name() const;
		void					SetName(const std::string& name);

		void					SetResourceConfig(ResourceConfig* pConfig);

	protected:
		static u32				s_usResourceUID;
		u32						m_usInnerID;

		std::string				m_name;

		ResourceConfig*			m_pResourceConfig = nullptr;
	};

	typedef std::shared_ptr<Resource> ResourcePtr;

};