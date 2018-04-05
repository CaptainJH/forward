#include "GeometryShaderDX11.h"
#include "dxCommon/ShaderFactoryDX.h"

using namespace forward;

GeometryShaderDX11::GeometryShaderDX11(ID3D11Device* device, forward::FrameGraphObject* obj)
	: ShaderDX11(obj)
{
	ShaderModel = L"gs_5_0";
	auto pShader = dynamic_cast<FrameGraphGeometryShader*>(obj);

	m_pCompiledShader = ShaderFactoryDX::GenerateShader(GEOMETRY_SHADER,
		pShader->GetShaderFile(), pShader->GetShaderEntry(), ShaderModel);
	assert(m_pCompiledShader);

	ID3D11ClassLinkage* linkage = nullptr;
	GeometryShaderComPtr dxShader;
	HR(device->CreateGeometryShader(m_pCompiledShader->GetBufferPointer(),
		m_pCompiledShader->GetBufferSize(), linkage, dxShader.GetAddressOf()));

	m_deviceObjPtr = dxShader;

	ReflectShader();
}

GeometryShaderDX11::~GeometryShaderDX11()
{
}

void GeometryShaderDX11::Bind(ID3D11DeviceContext* context)
{
	if (m_deviceObjPtr)
	{
		ID3D11ClassInstance* instances[1] = { nullptr };
		u32 numInstances = 0;
		ID3D11GeometryShader* dxShader = static_cast<ID3D11GeometryShader*>(m_deviceObjPtr.Get());
		context->GSSetShader(dxShader, instances, numInstances);
	}
}

void GeometryShaderDX11::Unbind(ID3D11DeviceContext* context)
{
	context->GSSetShader(nullptr, { nullptr }, 0);
}

void GeometryShaderDX11::BindCBuffer(ID3D11DeviceContext* context, u32 bindPoint, ID3D11Buffer* buffer)
{
	if (m_deviceObjPtr)
	{
		ID3D11Buffer* buffers[1] = { buffer };
		context->GSSetConstantBuffers(bindPoint, 1, buffers);
	}
}

void GeometryShaderDX11::UnbindCBuffer(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		context->GSSetConstantBuffers(bindPoint, 1, { nullptr });
	}
}

void GeometryShaderDX11::BindSRView(ID3D11DeviceContext* context, u32 bindPoint, ID3D11ShaderResourceView* srv)
{
	if (m_deviceObjPtr)
	{
		ID3D11ShaderResourceView* views[1] = { srv };
		context->GSSetShaderResources(bindPoint, 1, views);
	}
}

void GeometryShaderDX11::UnbindSRView(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		context->GSSetShaderResources(bindPoint, 1, { nullptr });
	}
}

void GeometryShaderDX11::BindSampler(ID3D11DeviceContext* context, u32 bindPoint, ID3D11SamplerState* state)
{
	if (m_deviceObjPtr)
	{
		ID3D11SamplerState* states[1] = { state };
		context->GSSetSamplers(bindPoint, 1, states);
	}
}

void GeometryShaderDX11::UnbindSampler(ID3D11DeviceContext* context, u32 bindPoint)
{
	if (m_deviceObjPtr)
	{
		context->GSSetSamplers(bindPoint, 1, { nullptr });
	}
}

