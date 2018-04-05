
//--------------------------------------------------------------------------------
#include "SamplerStateConfigDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
SamplerStateConfigDX11::SamplerStateConfigDX11()
{
	SetDefaults();
}
//--------------------------------------------------------------------------------
SamplerStateConfigDX11::~SamplerStateConfigDX11()
{
}
//--------------------------------------------------------------------------------
void SamplerStateConfigDX11::SetDefaults()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

	m_state.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_state.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_state.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_state.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_state.MipLODBias = 0.0f;
	m_state.MaxAnisotropy = 1;
	m_state.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	m_state.BorderColor[0] = 0.0f; 
	m_state.BorderColor[1] = 0.0f; 
	m_state.BorderColor[2] = 0.0f; 
	m_state.BorderColor[3] = 0.0f;
	m_state.MinLOD = 0;
	m_state.MaxLOD = D3D11_FLOAT32_MAX;
}
//--------------------------------------------------------------------------------
const D3D11_SAMPLER_DESC& SamplerStateConfigDX11::GetDesc() const
{
	return m_state;
}
//--------------------------------------------------------------------------------
D3D11_SAMPLER_DESC& SamplerStateConfigDX11::GetDesc()
{
	return m_state;
}