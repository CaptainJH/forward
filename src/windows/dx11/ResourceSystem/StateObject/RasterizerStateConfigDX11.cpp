//--------------------------------------------------------------------------------
#include "RasterizerStateConfigDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
RasterizerStateConfigDX11::RasterizerStateConfigDX11()
{
	SetDefaults();
}
//--------------------------------------------------------------------------------
RasterizerStateConfigDX11::~RasterizerStateConfigDX11()
{
}
//--------------------------------------------------------------------------------
void RasterizerStateConfigDX11::SetDefaults()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

	m_state.FillMode = D3D11_FILL_SOLID;
	m_state.CullMode = D3D11_CULL_BACK;
	m_state.FrontCounterClockwise = false;
	m_state.DepthBias = 0;
	m_state.SlopeScaledDepthBias = 0.0f;
	m_state.DepthBiasClamp = 0.0f;
	m_state.DepthClipEnable = true;
	m_state.ScissorEnable = false;
	m_state.MultisampleEnable = false;
	m_state.AntialiasedLineEnable = false;
}
//--------------------------------------------------------------------------------
const D3D11_RASTERIZER_DESC& RasterizerStateConfigDX11::GetDesc() const
{
	return m_state;
}
//--------------------------------------------------------------------------------
D3D11_RASTERIZER_DESC& RasterizerStateConfigDX11::GetDesc()
{
	return m_state;
}
//--------------------------------------------------------------------------------
RasterizerStateConfigDX11::RasterizerStateConfigDX11(const RasterizerState& rasterizerState)
{
	// Specify the rasterizer state description.
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = msFillMode[rasterizerState.fillMode];
	desc.CullMode = msCullMode[rasterizerState.cullMode];
	desc.FrontCounterClockwise = (rasterizerState.frontCCW ? TRUE : FALSE);
	desc.DepthBias = rasterizerState.depthBias;
	desc.DepthBiasClamp = rasterizerState.depthBiasClamp;
	desc.SlopeScaledDepthBias = rasterizerState.slopeScaledDepthBias;
	desc.DepthClipEnable = (rasterizerState.enableDepthClip ? TRUE : FALSE);
	desc.ScissorEnable = (rasterizerState.enableScissor ? TRUE : FALSE);
	desc.MultisampleEnable = (rasterizerState.enableMultisample ? TRUE : FALSE);
	desc.AntialiasedLineEnable = (rasterizerState.enableAntialiasedLine ? TRUE : FALSE);
}
//--------------------------------------------------------------------------------
D3D11_FILL_MODE const RasterizerStateConfigDX11::msFillMode[] =
{
	D3D11_FILL_SOLID,
	D3D11_FILL_WIREFRAME
};

D3D11_CULL_MODE const RasterizerStateConfigDX11::msCullMode[] =
{
	D3D11_CULL_NONE,
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK
};