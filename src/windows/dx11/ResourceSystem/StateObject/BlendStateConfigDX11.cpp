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
//--------------------------------------------------------------------------------
BlendStateConfigDX11::BlendStateConfigDX11(const BlendState& blendState)
{
	// Specify the blend state description.
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = (blendState.enableAlphaToCoverage ? TRUE : FALSE);
	desc.IndependentBlendEnable = (blendState.enableIndependentBlend ? TRUE : FALSE);
	for (u32 i = 0; i < BlendState::NUM_TARGETS; ++i)
	{
		D3D11_RENDER_TARGET_BLEND_DESC& out = desc.RenderTarget[i];
		BlendState::Target const& in = blendState.target[i];
		out.BlendEnable = (in.enable ? TRUE : FALSE);
		out.SrcBlend = msMode[in.srcColor];
		out.DestBlend = msMode[in.dstColor];
		out.BlendOp = msOperation[in.opColor];
		out.SrcBlendAlpha = msMode[in.srcAlpha];
		out.DestBlendAlpha = msMode[in.dstAlpha];
		out.BlendOpAlpha = msOperation[in.opAlpha];
		out.RenderTargetWriteMask = in.mask;
	}
}
//--------------------------------------------------------------------------------
D3D11_BLEND const BlendStateConfigDX11::msMode[] =
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_SRC_ALPHA_SAT,
	D3D11_BLEND_BLEND_FACTOR,
	D3D11_BLEND_INV_BLEND_FACTOR,
	D3D11_BLEND_SRC1_COLOR,
	D3D11_BLEND_INV_SRC1_COLOR,
	D3D11_BLEND_SRC1_ALPHA,
	D3D11_BLEND_INV_SRC1_ALPHA
};

D3D11_BLEND_OP const BlendStateConfigDX11::msOperation[] =
{
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX
};