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

	AlphaToCoverageEnable = false;
	IndependentBlendEnable = false;

	for ( int i = 0; i < 8; i++ )
	{
		RenderTarget[i].BlendEnable = false;
		RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
		RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD; 
		RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD; 
		RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
}
//--------------------------------------------------------------------------------
