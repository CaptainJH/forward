//***************************************************************************************
// DeviceDepthStencilStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceDepthStencilStateDX11.h"

using namespace forward;


ID3D11DepthStencilState * DeviceDepthStencilStateDX11::GetDepthStencilState()
{
	return static_cast<ID3D11DepthStencilState*>(m_deviceObjPtr.Get());
}

D3D11_DEPTH_WRITE_MASK const DeviceDepthStencilStateDX11::msWriteMask[] =
{
	D3D11_DEPTH_WRITE_MASK_ZERO,
	D3D11_DEPTH_WRITE_MASK_ALL
};

D3D11_COMPARISON_FUNC const DeviceDepthStencilStateDX11::msComparison[] =
{
	D3D11_COMPARISON_NEVER,
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_GREATER,
	D3D11_COMPARISON_NOT_EQUAL,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_ALWAYS
};

D3D11_STENCIL_OP const DeviceDepthStencilStateDX11::msOperation[] =
{
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_ZERO,
	D3D11_STENCIL_OP_REPLACE,
	D3D11_STENCIL_OP_INCR_SAT,
	D3D11_STENCIL_OP_DECR_SAT,
	D3D11_STENCIL_OP_INVERT,
	D3D11_STENCIL_OP_INCR,
	D3D11_STENCIL_OP_DECR
};