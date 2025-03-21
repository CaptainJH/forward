//***************************************************************************************
// DeviceBufferDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "dx11/ResourceSystem/DeviceResourceDX11.h"


namespace forward
{
	class DeviceBufferDX11 : public DeviceResourceDX11
	{
	public:
		DeviceBufferDX11(forward::GraphicsObject* obj);

		ID3D11Buffer*			GetDXBufferPtr();

		const D3D11_BUFFER_DESC&		GetActualDescription();

		u32						GetByteWidth();
		D3D11_USAGE				GetUsage();
		u32						GetBindFlags();
		u32						GetCPUAccessFlags();
		u32						GetMiscFlags();
		u32						GetStructureByteStride();

		void					SyncCPUToGPU() override;
		void					SyncCPUToGPU(ID3D11DeviceContext* context);
		void					SyncGPUToCPU(ID3D11DeviceContext* context);

	protected:
		D3D11_BUFFER_DESC		m_actualDesc;

		void					CreateStaging(ID3D11Device* device, const D3D11_BUFFER_DESC& descIn);
	};
}