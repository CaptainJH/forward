#include "ComputeShaderDX11.h"
#include "dxCommon/ShaderFactoryDX.h"

using namespace forward;

ComputeShaderDX11::ComputeShaderDX11(ID3D11Device* device, forward::FrameGraphObject* obj)
	: ShaderDX11(obj)
{
	ShaderModel = L"cs_5_0";
	auto pShader = dynamic_cast<FrameGraphComputeShader*>(obj);

	m_pCompiledShader = ShaderFactoryDX::GenerateShader(COMPUTE_SHADER,
		pShader->GetShaderFile(), pShader->GetShaderEntry(), ShaderModel);
	assert(m_pCompiledShader);

	ID3D11ClassLinkage* linkage = nullptr;
	ComputeShaderComPtr dxShader;
	HR(device->CreateComputeShader(m_pCompiledShader->GetBufferPointer(),
		m_pCompiledShader->GetBufferSize(), linkage, dxShader.GetAddressOf()));

	m_deviceObjPtr = dxShader;

	ReflectShader();
}

ComputeShaderDX11::~ComputeShaderDX11()
{}

void ComputeShaderDX11::Bind(ID3D11DeviceContext* context)
{
	if (m_deviceObjPtr)
	{
		ID3D11ClassInstance* instances[1] = { nullptr };
		u32 numInstances = 0;
		ID3D11ComputeShader* dxShader = static_cast<ID3D11ComputeShader*>(m_deviceObjPtr.Get());
		context->CSSetShader(dxShader, instances, numInstances);
	}
}

void ComputeShaderDX11::Unbind(ID3D11DeviceContext* context)
{
	ID3D11ClassInstance* instances[1] = { nullptr };
	u32 numInstances = 0;
	ID3D11ComputeShader* dxShader = nullptr;
	context->CSSetShader(dxShader, instances, numInstances);
}

void ComputeShaderDX11::BindCBuffer(ID3D11DeviceContext* context, u32 bindPoint, ID3D11Buffer* buffer)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { buffer };
		context->CSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void ComputeShaderDX11::UnbindCBuffer(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { nullptr };
		context->CSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void ComputeShaderDX11::BindSRView(ID3D11DeviceContext* context, u32 bindPoint, ID3D11ShaderResourceView* srv)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { srv };
		context->CSSetShaderResources(bindPoint, 1, views);
	}
}

void ComputeShaderDX11::UnbindSRView(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { nullptr };
		context->CSSetShaderResources(bindPoint, 1, views);
	}
}

void ComputeShaderDX11::BindSampler(ID3D11DeviceContext* context, u32 bindPoint, ID3D11SamplerState* state)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { state };
		context->CSSetSamplers(bindPoint, 1, states);
	}
}

void ComputeShaderDX11::UnbindSampler(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { nullptr };
		context->CSSetSamplers(bindPoint, 1, states);
	}
}