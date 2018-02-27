//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceObject.h"
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

	class DeviceResource : public DeviceObject
	{
	public:
		DeviceResource();

		virtual ~DeviceResource();

		virtual ResourceType	GetType() = 0;

		virtual u32				GetEvictionPriority() = 0;
		virtual void			SetEvictionPriority( u32 EvictionPriority ) = 0;

		virtual void			CopyCPUToGPU(u8* srcData, u32 srcDataSize) = 0;
		virtual void			CopyGPUToCPU(u8* dstData, u32 dstDataSize) = 0;
	};

	typedef std::shared_ptr<DeviceResource> ResourcePtr;

};