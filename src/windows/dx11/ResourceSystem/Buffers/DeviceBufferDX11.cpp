//***************************************************************************************
// DeviceBufferDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceBufferDX11.h"
#include "render/ResourceSystem/FrameGraphResource.h"

using namespace forward;

DeviceBufferDX11::DeviceBufferDX11(forward::FrameGraphObject* obj)
	: DeviceResourceDX11(obj)
{

}

ID3D11Buffer* DeviceBufferDX11::GetDXBufferPtr()
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
		GetDXBufferPtr()->GetDesc(&m_actualDesc);
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

void DeviceBufferDX11::SyncCPUToGPU(ID3D11DeviceContext* context)
{
	auto buffer = GetFrameGraphResource();
	if (!buffer || buffer->GetUsage() != ResourceUsage::RU_DYNAMIC_UPDATE)
	{
		return;
	}

	auto dxBuffer = GetDXBufferPtr();
	D3D11_MAPPED_SUBRESOURCE sub;
	HR(context->Map(dxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub));

	memcpy(sub.pData, buffer->GetData(), buffer->GetNumBytes());

	context->Unmap(dxBuffer, 0);
}

void DeviceBufferDX11::SyncCPUToGPU()
{
	/// TODO:
}