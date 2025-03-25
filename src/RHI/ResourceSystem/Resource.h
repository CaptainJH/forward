//***************************************************************************************
// Resource.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/GraphicsObject.h"
#include "RHI/ResourceSystem/DeviceResource.h"

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

	class Resource;
	class RWResourcePool
	{
	public:
		RWResourcePool() = default;
		template<class F> ResourcePtr FetchDeviceResource(u64 u, F&& failFunc);
		void ResetDeviceResource(u64 u);
		void Init(Resource* res) { m_resourcePtr = res; m_DeviceResPool.reserve(PoolSize); }

	private:
		typedef std::pair<u64, ResourcePtr> DeviceResourceEntryType;
		const static u32 PoolSize = 2;
		Vector<DeviceResourceEntryType> m_DeviceResPool;

		Resource* m_resourcePtr = nullptr;
	};

	class Resource : public GraphicsObject
	{
	public:
		Resource() = delete;
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
		u32 GetActiveNumElements() const;
		void ResetActiveNumElements();
		u32 GetElementSize() const;
		u32 GetNumBytes() const;

		void Initialize(u32 numElements, u32 elementSize);

		u8* GetData();
		template<class T> T GetData()
		{
			return reinterpret_cast<T>(GetData());
		}

		template<class F> ResourcePtr UpdateDynamicResource(u64 u, F&& failFunc)
		{
			if (GetUsage() == ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL ||
				GetUsage() == ResourceUsage::RU_DYNAMIC_UPDATE)
			{
				m_dynamicResPool.ResetDeviceResource(u);
				return m_dynamicResPool.FetchDeviceResource(u, failFunc);
			}
			else
				return nullptr;
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

		RWResourcePool m_dynamicResPool;
	};

	template<class F> 
	ResourcePtr RWResourcePool::FetchDeviceResource(u64 u, F&& failFunc)
	{
		if (!m_resourcePtr) return nullptr;
		assert(!m_resourcePtr->DeviceObject());
		ResourcePtr ret = nullptr;
		for (auto& pair : m_DeviceResPool)
			if (pair.first == 0)
			{
				ret = pair.second;
				pair.first = u;
				break;
			}
		if (!ret)
		{
			ret = failFunc();
			m_DeviceResPool.emplace_back(std::make_pair(u, ret));
		}
		m_resourcePtr->SetDeviceObject(ret);
		assert(m_resourcePtr->DeviceObject());
		return ret;
	}
}