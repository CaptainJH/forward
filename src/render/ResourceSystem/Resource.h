//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "GraphicsObject.h"
#include "render/ResourceSystem/DeviceResource.h"

namespace forward
{
	// The resource usage.  These control how the GPU versions are created.
	// You must set the usage type before binding the resource to an engine.
	enum ResourceUsage
	{
		RU_IMMUTABLE,       // D3D11_USAGE_IMMUTABLE (default)	GPU read
		RU_DYNAMIC_UPDATE,  // D3D11_USAGE_DYNAMIC				GPU read, CPU write
		RU_SHADER_OUTPUT,   // D3D11_USAGE_DEFAULT				GPU read/write
		RU_CPU_GPU_BIDIRECTIONAL, // D3D11_USAGE_STAGING		GPU read/write, CPU read/write
	};

	class Resource : public GraphicsObject
	{
	public:
		Resource();
		Resource(const std::string& name);
		virtual ~Resource();

		DeviceResource*		GetDeviceResource();

		void SetUsage(ResourceUsage usage);
		ResourceUsage GetUsage() const;

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

		u8* GetData();
		template<class T> T GetData()
		{
			return reinterpret_cast<T>(GetData());
		}

		static void CopyPitched2(u32 numRows, u32 srcRowPitch, const u8* srcData, u32 dstRowPitch, u8* dstData);
		static void CopyPitched3(u32 numRows, u32 numSlices, u32 srcRowPitch, u32 srcSlicePitch, const u8* srcData,
			u32 dstRowPitch, u32 dstSlicePitch, u8* dstData);

	protected:

		u32		m_numElements;
		u32		m_numActiveElements;
		u32		m_elementSize;
		u32		m_numBytes;
		std::vector<u8> m_storage;
		u8*		m_data;
		ResourceUsage	m_usage;
	};
}