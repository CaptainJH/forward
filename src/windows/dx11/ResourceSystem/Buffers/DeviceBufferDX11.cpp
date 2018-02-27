//***************************************************************************************
// DeviceBufferDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceBufferDX11.h"

using namespace forward;

ID3D11Buffer* DeviceBufferDX11::GetBufferPtr()
{
	if (m_deviceResPtr)
	{
		return static_cast<ID3D11Buffer*>(m_deviceResPtr.Get());
	}

	return nullptr;
}

const D3D11_BUFFER_DESC& DeviceBufferDX11::GetActualDescription()
{
	ZeroMemory(&m_actualDesc, sizeof(D3D11_BUFFER_DESC));

	if (m_deviceResPtr)
	{
		GetBufferPtr()->GetDesc(&m_actualDesc);
	}

	return m_actualDesc;
}

u32 DeviceBufferDX11::GetByteWidth()
{
	auto description = GetActualDescription();

	return description.ByteWidth;
}

D3D11_USAGE DeviceBufferDX11::GetUsage()
{
	auto description = GetActualDescription();

	return description.Usage;
}

u32 DeviceBufferDX11::GetBindFlags()
{
	auto description = GetActualDescription();

	return description.BindFlags;
}

u32 DeviceBufferDX11::GetCPUAccessFlags()
{
	auto description = GetActualDescription();

	return description.CPUAccessFlags;
}

u32 DeviceBufferDX11::GetMiscFlags()
{
	auto description = GetActualDescription();

	return description.MiscFlags;
}

u32 DeviceBufferDX11::GetStructureByteStride()
{
	auto description = GetActualDescription();

	return description.StructureByteStride;
}

void DeviceBufferDX11::CopyCPUToGPU(u8* /*srcData*/, u32 /*srcDataSize*/)
{
	/// TODO: 
}

void DeviceBufferDX11::CopyGPUToCPU(u8* /*dstData*/, u32 /*dstDataSize*/)
{
	/// TODO:
}