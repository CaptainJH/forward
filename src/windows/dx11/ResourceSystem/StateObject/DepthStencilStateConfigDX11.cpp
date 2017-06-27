//--------------------------------------------------------------------------------
#include "DepthStencilStateConfigDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
DepthStencilStateConfigDX11::DepthStencilStateConfigDX11()
{
	SetDefaults();
}
//--------------------------------------------------------------------------------
DepthStencilStateConfigDX11::~DepthStencilStateConfigDX11()
{
}
//--------------------------------------------------------------------------------
void DepthStencilStateConfigDX11::SetDefaults()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

	m_state.DepthEnable = true;
	m_state.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m_state.DepthFunc = D3D11_COMPARISON_LESS;
	m_state.StencilEnable = FALSE;
	m_state.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	m_state.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		
	m_state.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	m_state.FrontFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	m_state.FrontFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	m_state.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	
	m_state.BackFace.StencilFunc			= D3D11_COMPARISON_NEVER;
	m_state.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	m_state.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	m_state.BackFace.StencilDepthFailOp		= D3D11_STENCIL_OP_KEEP;
}
//--------------------------------------------------------------------------------
const D3D11_DEPTH_STENCIL_DESC& DepthStencilStateConfigDX11::GetDesc() const
{
	return m_state;
}
//--------------------------------------------------------------------------------
D3D11_DEPTH_STENCIL_DESC& DepthStencilStateConfigDX11::GetDesc()
{
	return m_state;
}
//--------------------------------------------------------------------------------
DepthStencilStateConfigDX11::DepthStencilStateConfigDX11(const DepthStencilState& depthStencilState)
{
	// Specify the rasterizer state description.
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = (depthStencilState.depthEnable ? TRUE : FALSE);
	desc.DepthWriteMask = msWriteMask[depthStencilState.writeMask];
	desc.DepthFunc = msComparison[depthStencilState.comparison];
	desc.StencilEnable = (depthStencilState.stencilEnable ? TRUE : FALSE);
	desc.StencilReadMask = depthStencilState.stencilReadMask;
	desc.StencilWriteMask = depthStencilState.stencilWriteMask;
	DepthStencilState::Face front = depthStencilState.frontFace;
	desc.FrontFace.StencilFailOp = msOperation[front.fail];
	desc.FrontFace.StencilDepthFailOp = msOperation[front.depthFail];
	desc.FrontFace.StencilPassOp = msOperation[front.pass];
	desc.FrontFace.StencilFunc = msComparison[front.comparison];
	DepthStencilState::Face back = depthStencilState.backFace;
	desc.BackFace.StencilFailOp = msOperation[back.fail];
	desc.BackFace.StencilDepthFailOp = msOperation[back.depthFail];
	desc.BackFace.StencilPassOp = msOperation[back.pass];
	desc.BackFace.StencilFunc = msComparison[back.comparison];
}
//--------------------------------------------------------------------------------
D3D11_DEPTH_WRITE_MASK const DepthStencilStateConfigDX11::msWriteMask[] =
{
	D3D11_DEPTH_WRITE_MASK_ZERO,
	D3D11_DEPTH_WRITE_MASK_ALL
};

D3D11_COMPARISON_FUNC const DepthStencilStateConfigDX11::msComparison[] =
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

D3D11_STENCIL_OP const DepthStencilStateConfigDX11::msOperation[] =
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