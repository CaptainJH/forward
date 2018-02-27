//***************************************************************************************
// DeviceRasterizerStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceRasterizerStateDX11.h"

using namespace forward;

ID3D11RasterizerState* DeviceRasterizerStateDX11::GetRasterizerState()
{
	return static_cast<ID3D11RasterizerState*>(m_deviceObjPtr.Get());
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