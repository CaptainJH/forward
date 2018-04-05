//***************************************************************************************
// DeviceInputLayout.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceInputLayoutDX11.h"
#include "dxCommon/ShaderDX.h"

using namespace forward;

InputLayoutComPtr	DeviceInputLayoutDX11::GetInputLayout()
{
	return m_inputLayout;
}

DeviceInputLayoutDX11::DeviceInputLayoutDX11(ID3D11Device* device, const FrameGraphVertexBuffer* vbuffer, FrameGraphVertexShader* vshader)
	: DeviceObject(const_cast<VertexFormat*>(&vbuffer->GetVertexFormat()))
	, m_numElements(0)
	, m_inputLayout(nullptr)
{
	ZeroMemory(&m_elements[0], VA_MAX_ATTRIBUTES * sizeof(m_elements[0]));

	if (vbuffer && vshader)
	{
		const auto& vertexFormat = vbuffer->GetVertexFormat();
		m_numElements = vertexFormat.GetNumAttributes();
		for (auto i = 0U; i < m_numElements; ++i)
		{
			VASemantic semantic;
			DataFormatType type;
			u32 unit, offset;
			vertexFormat.GetAttribute(i, semantic, type, unit, offset);

			D3D11_INPUT_ELEMENT_DESC& element = m_elements[i];
			element.SemanticName = msSemantic[semantic];
			element.SemanticIndex = unit;
			element.Format = static_cast<DXGI_FORMAT>(type);
			element.InputSlot = 0;  // TODO: Streams not yet supported.
			element.AlignedByteOffset = offset;
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
		}

		auto compiledCode = device_cast<ShaderDX*>(vshader)->GetCompiledCode();
		HR(device->CreateInputLayout(m_elements, m_numElements, compiledCode->GetBufferPointer(), 
			compiledCode->GetBufferSize(), m_inputLayout.GetAddressOf()));

	}
}

DeviceInputLayoutDX11::~DeviceInputLayoutDX11()
{

}

void DeviceInputLayoutDX11::Bind(ID3D11DeviceContext* deviceContext)
{
	if (m_inputLayout)
	{
		deviceContext->IASetInputLayout(m_inputLayout.Get());
	}
}

void DeviceInputLayoutDX11::Unbind(ID3D11DeviceContext* deviceContext)
{
	if (m_inputLayout)
	{
		deviceContext->IASetInputLayout(nullptr);
	}
}

i8 const* DeviceInputLayoutDX11::msSemantic[VA_NUM_SEMANTICS] =
{
	"",
	"POSITION",
	"BLENDWEIGHT",
	"BLENDINDICES",
	"NORMAL",
	"PSIZE",
	"TEXCOORD",
	"TANGENT",
	"BINORMAL",
	"TESSFACTOR",
	"POSITIONT",
	"COLOR",
	"FOG",
	"DEPTH",
	"SAMPLE"
};