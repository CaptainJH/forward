#include "VertexShaderDX11.h"
#include "dxCommon/ShaderFactoryDX.h"

using namespace forward;

VertexShaderDX11::VertexShaderDX11(ID3D11Device* device, forward::FrameGraphObject* obj)
	: ShaderDX11(obj)
{
	ShaderModel = L"vs_5_0";
	auto pShader = dynamic_cast<FrameGraphVertexShader*>(obj);

	m_pCompiledShader = ShaderFactoryDX::GenerateShader(VERTEX_SHADER, 
		pShader->GetShaderFile(), pShader->GetShaderEntry(), ShaderModel);
	assert(m_pCompiledShader);

	ID3D11ClassLinkage* linkage = nullptr;
	VertexShaderComPtr dxShader;
	HR(device->CreateVertexShader(m_pCompiledShader->GetBufferPointer(),
		m_pCompiledShader->GetBufferSize(), linkage, dxShader.GetAddressOf()));

	m_deviceObjPtr = dxShader;

	ReflectShader();
}

VertexShaderDX11::~VertexShaderDX11()
{}

void VertexShaderDX11::Bind(ID3D11DeviceContext* context)
{
	if (m_deviceObjPtr)
	{
		ID3D11ClassInstance* instances[1] = { nullptr };
		u32 numInstances = 0;
		ID3D11VertexShader* dxShader = static_cast<ID3D11VertexShader*>(m_deviceObjPtr.Get());
		context->VSSetShader(dxShader, instances, numInstances);
	}
}

void VertexShaderDX11::Unbind(ID3D11DeviceContext* context)
{
	ID3D11ClassInstance* instances[1] = { nullptr };
	u32 numInstances = 0;
	ID3D11VertexShader* dxShader = nullptr;
	context->VSSetShader(dxShader, instances, numInstances);
}

void VertexShaderDX11::BindCBuffer(ID3D11DeviceContext* context, u32 bindPoint, ID3D11Buffer* buffer)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { buffer };
		context->VSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void VertexShaderDX11::UnbindCBuffer(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { nullptr };
		context->VSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void VertexShaderDX11::BindSRView(ID3D11DeviceContext* context, u32 bindPoint, ID3D11ShaderResourceView* srv)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { srv };
		context->VSSetShaderResources(bindPoint, 1, views);
	}
}

void VertexShaderDX11::UnbindSRView(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { nullptr };
		context->VSSetShaderResources(bindPoint, 1, views);
	}
}

void VertexShaderDX11::BindSampler(ID3D11DeviceContext* context, u32 bindPoint, ID3D11SamplerState* state)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { state };
		context->VSSetSamplers(bindPoint, 1, states);
	}
}

void VertexShaderDX11::UnbindSampler(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { nullptr };
		context->VSSetSamplers(bindPoint, 1, states);
	}
}