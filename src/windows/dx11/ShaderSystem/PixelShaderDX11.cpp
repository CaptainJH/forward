#include "PixelShaderDX11.h"
#include "dxCommon/ShaderFactoryDX.h"

using namespace forward;

PixelShaderDX11::PixelShaderDX11(ID3D11Device* device, forward::FrameGraphObject* obj)
	: ShaderDX11(obj)
{
	ShaderModel = L"ps_5_0";
	auto pShader = dynamic_cast<FrameGraphPixelShader*>(obj);

	m_pCompiledShader = ShaderFactoryDX::GenerateShader(PIXEL_SHADER,
		pShader->GetShaderFile(), pShader->GetShaderEntry(), ShaderModel);
	assert(m_pCompiledShader);

	ID3D11ClassLinkage* linkage = nullptr;
	PixelShaderComPtr dxShader;
	HR(device->CreatePixelShader(m_pCompiledShader->GetBufferPointer(),
		m_pCompiledShader->GetBufferSize(), linkage, dxShader.GetAddressOf()));

	m_deviceObjPtr = dxShader;

	ReflectShader();
}

PixelShaderDX11::~PixelShaderDX11()
{}

void PixelShaderDX11::Bind(ID3D11DeviceContext* context)
{
	if (m_deviceObjPtr)
	{
		ID3D11ClassInstance* instances[1] = { nullptr };
		u32 numInstances = 0;
		ID3D11PixelShader* dxShader = static_cast<ID3D11PixelShader*>(m_deviceObjPtr.Get());
		context->PSSetShader(dxShader, instances, numInstances);
	}
}

void PixelShaderDX11::Unbind(ID3D11DeviceContext* context)
{
	if (m_deviceObjPtr)
	{
		ID3D11ClassInstance* instances[1] = { nullptr };
		u32 numInstances = 0;
		ID3D11PixelShader* dxShader = nullptr;
		context->PSSetShader(dxShader, instances, numInstances);
	}
}

void PixelShaderDX11::BindCBuffer(ID3D11DeviceContext* context, u32 bindPoint, ID3D11Buffer* buffer)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { buffer };
		context->PSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void PixelShaderDX11::UnbindCBuffer(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { nullptr };
		context->PSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void PixelShaderDX11::BindSRView(ID3D11DeviceContext* context, u32 bindPoint, ID3D11ShaderResourceView* srv)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { srv };
		context->PSSetShaderResources(bindPoint, 1, views);
	}
}

void PixelShaderDX11::UnbindSRView(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { nullptr };
		context->PSSetShaderResources(bindPoint, 1, views);
	}
}

void PixelShaderDX11::BindSampler(ID3D11DeviceContext* context, u32 bindPoint, ID3D11SamplerState* state)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { state };
		context->PSSetSamplers(bindPoint, 1, states);
	}
}

void PixelShaderDX11::UnbindSampler(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { nullptr };
		context->PSSetSamplers(bindPoint, 1, states);
	}
}