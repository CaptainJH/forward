//***************************************************************************************
// DeviceVertexBufferDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceVertexBufferDX11.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"

using namespace forward;

ResourceType DeviceVertexBufferDX11::GetType()
{
	return RT_VERTEXBUFFER;
}

DeviceVertexBufferDX11::DeviceVertexBufferDX11(ID3D11Device* device, FrameGraphVertexBuffer* vb)
	: DeviceBufferDX11(vb)
{
	// Specify the buffer description.
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = vb->GetNumBytes();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
	desc.StructureByteStride = 0;
	ResourceUsage usage = vb->GetUsage();
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
	else // ResourceUsage::RU_SHADER_OUTPUT
	{
		Log::Get().Write(L"Vertex output streams are not yet tested.");
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}

	// Create the buffer
	BufferComPtr buffer;
	if (vb->GetData())
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = vb->GetData();
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

void DeviceVertexBufferDX11::Bind(ID3D11DeviceContext* context)
{
	if (m_deviceResPtr)
	{
		// The MSDN documentation for ID3D11DeviceContext::IASetVertexBuffers
		// and ID3D11DeviceContext::Draw(numVertices, startVertex) appears
		// not to mention that startVertex is relative to the offsets[]
		// passed to IASetVertexBuffers.  If you set the offsets[0] here, you
		// should call Draw(numVertices,0).  If you instead call
		// Draw(numVertices, startVertex), then you should set offsets[0]
		// to 0.  The latter choice is made for GTEngine.  TODO:  Is there a
		// performance issue by setting offsets[0] to zero?  This depends on
		// what the input assembly stage does with the buffers when you
		// enable them using IASetVertexBuffers.
		ID3D11Buffer* buffers[1] = { GetDXBufferPtr() };
		u32 strides[1] = { GetFrameGraphResource()->GetElementSize() };
		u32 offsets[1] = { 0 };
		context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	}
}

void DeviceVertexBufferDX11::Unbind(ID3D11DeviceContext* context)
{
	if (m_deviceResPtr)
	{
		ID3D11Buffer* buffers[1] = { nullptr };
		u32 strides[1] = { 0 };
		u32 offsets[1] = { 0 };
		context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	}
}