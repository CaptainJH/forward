//--------------------------------------------------------------------------------
#include "Texture2dConfigDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Texture2dConfigDX11::Texture2dConfigDX11()
{
	SetDefaults();
}
//--------------------------------------------------------------------------------
Texture2dConfigDX11::~Texture2dConfigDX11()
{
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetDefaults()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

    m_State.Width = 1;
	m_State.Height = 1;
    m_State.MipLevels = 1;
    m_State.ArraySize = 1;
    m_State.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	m_State.SampleDesc.Count = 1;
	m_State.SampleDesc.Quality = 0;
    m_State.Usage = D3D11_USAGE_DEFAULT;
    m_State.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    m_State.CPUAccessFlags = 0;
    m_State.MiscFlags = 0;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetDepthBuffer(forward::UINT width, forward::UINT height)
{
	m_State.Width = width;
	m_State.Height = height;
	m_State.MipLevels = 1;
	m_State.ArraySize = 1;
	m_State.Format = DXGI_FORMAT_D32_FLOAT;
	m_State.SampleDesc.Count = 1;
	m_State.SampleDesc.Quality = 0;
	m_State.Usage = D3D11_USAGE_DEFAULT;
	m_State.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	m_State.CPUAccessFlags = 0;
	m_State.MiscFlags = 0;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetColorBuffer(forward::UINT width, forward::UINT height)
{
	m_State.Width = width;
	m_State.Height = height;
	m_State.MipLevels = 1;
	m_State.ArraySize = 1;
	m_State.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_State.SampleDesc.Count = 1;
	m_State.SampleDesc.Quality = 0;
	m_State.Usage = D3D11_USAGE_DEFAULT;
	m_State.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	m_State.CPUAccessFlags = 0;
	m_State.MiscFlags = 0;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetWidth( forward::UINT state )
{
	m_State.Width = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetHeight( forward::UINT state )
{
	m_State.Height = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetMipLevels( forward::UINT state )
{
	m_State.MipLevels = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetArraySize( forward::UINT state )
{
	m_State.ArraySize = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetFormat( DXGI_FORMAT state )
{
	m_State.Format = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetSampleDesc( DXGI_SAMPLE_DESC state )
{
	m_State.SampleDesc = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetUsage( D3D11_USAGE state ) 
{
	m_State.Usage = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetBindFlags( forward::UINT state )
{
	m_State.BindFlags = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetCPUAccessFlags( forward::UINT state )
{
	m_State.CPUAccessFlags = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetMiscFlags( forward::UINT state )
{
	m_State.MiscFlags = state;
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE2D_DESC Texture2dConfigDX11::GetTextureDesc()
{
	return( m_State );
}
//--------------------------------------------------------------------------------