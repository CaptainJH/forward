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

		ID3D11Buffer*			GetBufferPtr();

		const D3D11_BUFFER_DESC&		GetActualDescription();

		u32						GetByteWidth();
		D3D11_USAGE				GetUsage();
		u32						GetBindFlags();
		u32						GetCPUAccessFlags();
		u32						GetMiscFlags();
		u32						GetStructureByteStride();

		void					CopyCPUToGPU(u8* srcData, u32 srcDataSize) override;
		void					CopyGPUToCPU(u8* dstData, u32 dstDataSize) override;

	protected:
		D3D11_BUFFER_DESC		m_actualDesc;
	};
}