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