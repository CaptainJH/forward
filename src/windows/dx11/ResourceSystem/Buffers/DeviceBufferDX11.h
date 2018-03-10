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
		DeviceBufferDX11(forward::FrameGraphObject* obj);

		ID3D11Buffer*			GetDXBufferPtr();

		const D3D11_BUFFER_DESC&		GetActualDescription();

		u32						GetByteWidth();
		D3D11_USAGE				GetUsage();
		u32						GetBindFlags();
		u32						GetCPUAccessFlags();
		u32						GetMiscFlags();
		u32						GetStructureByteStride();

		void					SyncCPUToGPU() override;

	private:
		void					SyncCPUToGPU(ID3D11DeviceContext* context);

	protected:
		D3D11_BUFFER_DESC		m_actualDesc;
	};
}