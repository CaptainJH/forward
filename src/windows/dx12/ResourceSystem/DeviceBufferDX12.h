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
		DeviceBufferDX12(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, forward::GraphicsObject* obj);
		virtual ~DeviceBufferDX12();

		//ID3D11Buffer*			GetDXBufferPtr();

		//const D3D11_BUFFER_DESC&		GetActualDescription();

		//u32						GetByteWidth();
		//D3D11_USAGE				GetUsage();
		//u32						GetBindFlags();
		//u32						GetCPUAccessFlags();
		//u32						GetMiscFlags();
		//u32						GetStructureByteStride();

		D3D12_VERTEX_BUFFER_VIEW		VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW		IndexBufferView();

		D3D12_CPU_DESCRIPTOR_HANDLE	GetCBViewCPUHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE GetCBViewGPUHandle();
		void SetCBViewGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE inHandle);

		void					SyncCPUToGPU() override;
		void					SyncCPUToGPU(ID3D12GraphicsCommandList* cmdList);
		//void					SyncGPUToCPU(ID3D11DeviceContext* context);

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE		m_cbvHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE		m_cbvHandleGPU;
		D3D12_CPU_DESCRIPTOR_HANDLE		m_uavHandle;

		u8*														m_mappedData = nullptr;

		void CreateCBView(ID3D12Device* device, const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
	};
}