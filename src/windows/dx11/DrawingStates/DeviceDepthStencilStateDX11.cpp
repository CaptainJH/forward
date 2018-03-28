//***************************************************************************************
// DeviceDepthStencilStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceDepthStencilStateDX11.h"

using namespace forward;

DeviceDepthStencilStateDX11::DeviceDepthStencilStateDX11(ID3D11Device* device, DepthStencilState* dsState)
	: DeviceDrawingStateDX11(dsState)
{
	// Specify the rasterizer state description.
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = (dsState->depthEnable ? TRUE : FALSE);
	desc.DepthWriteMask = msWriteMask[dsState->writeMask];
	desc.DepthFunc = msComparison[dsState->comparison];
	desc.StencilEnable = (dsState->stencilEnable ? TRUE : FALSE);
	desc.StencilReadMask = dsState->stencilReadMask;
	desc.StencilWriteMask = dsState->stencilWriteMask;
	DepthStencilState::Face front = dsState->frontFace;
	desc.FrontFace.StencilFailOp = msOperation[front.fail];
	desc.FrontFace.StencilDepthFailOp = msOperation[front.depthFail];
	desc.FrontFace.StencilPassOp = msOperation[front.pass];
	desc.FrontFace.StencilFunc = msComparison[front.comparison];
	DepthStencilState::Face back = dsState->backFace;
	desc.BackFace.StencilFailOp = msOperation[back.fail];
	desc.BackFace.StencilDepthFailOp = msOperation[back.depthFail];
	desc.BackFace.StencilPassOp = msOperation[back.pass];
	desc.BackFace.StencilFunc = msComparison[back.comparison];

	// Create the depth-stencil state.
	ID3D11DepthStencilState* state = nullptr;
	HR(device->CreateDepthStencilState(&desc, &state));
	m_deviceObjPtr = state;
}

DeviceDepthStencilStateDX11::~DeviceDepthStencilStateDX11()
{
}

void DeviceDepthStencilStateDX11::Bind(ID3D11DeviceContext* deviceContext)
{
	auto dsState = GetDepthStencilState();
	deviceContext->OMSetDepthStencilState(GetDepthStencilStateDX11(), dsState->reference);
}

ID3D11DepthStencilState * DeviceDepthStencilStateDX11::GetDepthStencilStateDX11()
{
	return static_cast<ID3D11DepthStencilState*>(m_deviceObjPtr.Get());
}

shared_ptr<DepthStencilState> DeviceDepthStencilStateDX11::GetDepthStencilState()
{
	return m_frameGraphObjPtr.lock();
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