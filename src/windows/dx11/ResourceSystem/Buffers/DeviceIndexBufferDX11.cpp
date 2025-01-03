//***************************************************************************************
// DeviceIndexBufferDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceIndexBufferDX11.h"
#include "render/ResourceSystem/Buffer.h"

using namespace forward;

DeviceIndexBufferDX11::DeviceIndexBufferDX11(ID3D11Device* device, IndexBuffer* ib)
	: DeviceBufferDX11(ib)
	, m_format(ib->GetElementSize() == sizeof(u32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT)
{
	// Specify the buffer description.
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = ib->GetNumBytes();
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
	desc.StructureByteStride = 0;
	ResourceUsage usage = ib->GetUsage();
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
	if (ib->GetData())
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = ib->GetData();
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

void DeviceIndexBufferDX11::Bind(ID3D11DeviceContext* context)
{
	if (m_deviceResPtr)
	{
		auto dxBuffer = GetDXBufferPtr();
		context->IASetIndexBuffer(dxBuffer, m_format, 0);
	}
}

void DeviceIndexBufferDX11::Unbind(ID3D11DeviceContext* context)
{
	if (m_deviceResPtr)
	{
		context->IASetIndexBuffer(0, DXGI_FORMAT_UNKNOWN, 0);
	}
}