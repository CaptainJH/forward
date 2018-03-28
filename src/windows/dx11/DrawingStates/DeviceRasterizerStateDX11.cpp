//***************************************************************************************
// DeviceRasterizerStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceRasterizerStateDX11.h"

using namespace forward;

DeviceRasterizerStateDX11::DeviceRasterizerStateDX11(ID3D11Device* device, RasterizerState* rsState)
	: DeviceDrawingStateDX11(rsState)
{
	// Specify the rasterizer state description.
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = msFillMode[rsState->fillMode];
	desc.CullMode = msCullMode[rsState->cullMode];
	desc.FrontCounterClockwise = (rsState->frontCCW ? TRUE : FALSE);
	desc.DepthBias = rsState->depthBias;
	desc.DepthBiasClamp = rsState->depthBiasClamp;
	desc.SlopeScaledDepthBias = rsState->slopeScaledDepthBias;
	desc.DepthClipEnable = (rsState->enableDepthClip ? TRUE : FALSE);
	desc.ScissorEnable = (rsState->enableScissor ? TRUE : FALSE);
	desc.MultisampleEnable = (rsState->enableMultisample ? TRUE : FALSE);
	desc.AntialiasedLineEnable = (rsState->enableAntialiasedLine ? TRUE : FALSE);

	// Create the rasterizer state.
	RasterizerStateComPtr state;
	HR(device->CreateRasterizerState(&desc, state.GetAddressOf()));
	m_deviceObjPtr = state;
}

DeviceRasterizerStateDX11::~DeviceRasterizerStateDX11()
{
}

ID3D11RasterizerState* DeviceRasterizerStateDX11::GetRasterizerStateDX11()
{
	return static_cast<ID3D11RasterizerState*>(m_deviceObjPtr.Get());
}

shared_ptr<RasterizerState> DeviceRasterizerStateDX11::GetRasterizerState()
{
	return m_frameGraphObjPtr.lock();
}

void DeviceRasterizerStateDX11::Bind(ID3D11DeviceContext* deviceContext)
{
	deviceContext->RSSetState(GetRasterizerStateDX11());
}

D3D11_FILL_MODE const DeviceRasterizerStateDX11::msFillMode[] =
{
	D3D11_FILL_SOLID,
	D3D11_FILL_WIREFRAME
};

D3D11_CULL_MODE const DeviceRasterizerStateDX11::msCullMode[] =
{
	D3D11_CULL_NONE,
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK
};