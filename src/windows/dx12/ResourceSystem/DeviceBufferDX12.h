//***************************************************************************************
// DeviceBufferDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx12/ResourceSystem/DeviceResourceDX12.h"


namespace forward
{
	class DeviceBufferDX12 : public DeviceResourceDX12
	{
	public:
		DeviceBufferDX12(ID3D12Device* device, forward::FrameGraphObject* obj);

		//ID3D11Buffer*			GetDXBufferPtr();

		//const D3D11_BUFFER_DESC&		GetActualDescription();

		//u32						GetByteWidth();
		//D3D11_USAGE				GetUsage();
		//u32						GetBindFlags();
		//u32						GetCPUAccessFlags();
		//u32						GetMiscFlags();
		//u32						GetStructureByteStride();

		void					SyncCPUToGPU() override;
		//void					SyncCPUToGPU(ID3D11DeviceContext* context);
		//void					SyncGPUToCPU(ID3D11DeviceContext* context);

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE		m_uavHandle;

		//void					CreateStaging(ID3D11Device* device, const D3D11_BUFFER_DESC& descIn);
	};
}