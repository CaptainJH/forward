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

	FillMode = D3D11_FILL_SOLID;
	CullMode = D3D11_CULL_BACK;
	FrontCounterClockwise = false;
	DepthBias = 0;
	SlopeScaledDepthBias = 0.0f;
	DepthBiasClamp = 0.0f;
	DepthClipEnable = true;
	ScissorEnable = false;
	MultisampleEnable = false;
	AntialiasedLineEnable = false;
}
//--------------------------------------------------------------------------------
