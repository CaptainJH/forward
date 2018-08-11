//***************************************************************************************
// DeviceConstantBufferDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceConstantBufferDX11.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"

using namespace forward;

DeviceConstantBufferDX11::DeviceConstantBufferDX11(ID3D11Device* device, FrameGraphConstantBufferBase* cb)
	: DeviceBufferDX11(cb)
{
	// Specify the buffer description.
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = cb->GetNumBytes();
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
	desc.StructureByteStride = 0;
	ResourceUsage usage = cb->GetUsage();
	if (usage == ResourceUsage::RU_IMMUTABLE)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}
	else if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}

	// Create the buffer.
	BufferComPtr buffer;
	if (cb->GetData())
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = cb->GetData();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		HR(device->CreateBuffer(&desc, &data, buffer.GetAddressOf()));
	}
	else
	{
		HR(device->CreateBuffer(&desc, nullptr, buffer.GetAddressOf()));
	}

	m_deviceResPtr = buffer;
}