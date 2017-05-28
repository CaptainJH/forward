//--------------------------------------------------------------------------------
#include "BlendStateConfigDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
BlendStateConfigDX11::BlendStateConfigDX11()
{
	SetDefaults();
}
//--------------------------------------------------------------------------------
BlendStateConfigDX11::~BlendStateConfigDX11()
{
}
//--------------------------------------------------------------------------------
void BlendStateConfigDX11::SetDefaults()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

	m_state.AlphaToCoverageEnable = false;
	m_state.IndependentBlendEnable = false;

	for ( auto i = 0; i < 8; i++ )
	{
		m_state.RenderTarget[i].BlendEnable = false;
		m_state.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		m_state.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
		m_state.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD; 
		m_state.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		m_state.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		m_state.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD; 
		m_state.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
}
//--------------------------------------------------------------------------------
const D3D11_BLEND_DESC& BlendStateConfigDX11::GetDesc() const
{
	return m_state;
}
//--------------------------------------------------------------------------------
D3D11_BLEND_DESC& BlendStateConfigDX11::GetDesc()
{
	return m_state;
}