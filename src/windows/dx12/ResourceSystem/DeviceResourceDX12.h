//***************************************************************************************
// DeviceResourceDX12.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "RHI/ResourceSystem/DeviceResource.h"
#include "dx12/dx12Util.h"

namespace forward
{
	class DeviceDX12;
	class DeviceResourceDX12 : public DeviceResource
	{
	public:
		DeviceResourceDX12(forward::GraphicsObject* obj, DeviceDX12& d);

		virtual ~DeviceResourceDX12();

		DeviceResCom12Ptr		GetDeviceResource();

		u32					GetEvictionPriority() override;
		void				SetEvictionPriority(u32 EvictionPriority) override;

		void										SetResourceState(D3D12_RESOURCE_STATES state);
		D3D12_RESOURCE_STATES	GetResourceState() const;
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return m_gpuVirtualAddress; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceViewHandle();

	protected:
		DeviceResCom12Ptr		m_deviceResPtr;
		DeviceResCom12Ptr		m_stagingResPtr;

		D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHandle;

		D3D12_RESOURCE_STATES				m_currentUsageState;
		//D3D12_RESOURCE_STATES				m_transitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS			m_gpuVirtualAddress;
		DeviceDX12& m_device;

		bool				PrepareForSync();
	};
}
