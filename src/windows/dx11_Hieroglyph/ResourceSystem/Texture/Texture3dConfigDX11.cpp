//--------------------------------------------------------------------------------
#include "Texture3dConfigDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Texture3dConfigDX11::Texture3dConfigDX11()
{
	SetDefaults();
}
//--------------------------------------------------------------------------------
Texture3dConfigDX11::Texture3dConfigDX11(const Texture3dConfig& config)
{
	SetDefaults();

	auto width = config.GetWidth();
	auto height = config.GetHeight();
	auto depth = config.GetDepth();
	auto format = config.GetFormat();

	m_State.Width = width;
	m_State.Height = height;
	m_State.Depth = depth;
	m_State.Format = static_cast<DXGI_FORMAT>(format);
}
//--------------------------------------------------------------------------------
Texture3dConfigDX11::~Texture3dConfigDX11()
{
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetDefaults()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

    m_State.Width = 1;
    m_State.Height = 1;
    m_State.Depth = 1;
    m_State.MipLevels = 1;
    m_State.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    m_State.Usage = D3D11_USAGE_DEFAULT;
    m_State.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    m_State.CPUAccessFlags = 0;
    m_State.MiscFlags = 0;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetWidth( u32 state )
{
	m_State.Width = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetHeight( u32 state )
{
	m_State.Height = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetDepth( u32 state )
{
	m_State.Depth = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetMipLevels( u32 state )
{
	m_State.MipLevels = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetFormat( DXGI_FORMAT state )
{
	m_State.Format = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetUsage( D3D11_USAGE state ) 
{
	m_State.Usage = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetBindFlags( u32 state )
{
	m_State.BindFlags = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetCPUAccessFlags( u32 state )
{
	m_State.CPUAccessFlags = state;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::SetMiscFlags( u32 state )
{
	m_State.MiscFlags = state;
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE3D_DESC Texture3dConfigDX11::GetTextureDesc()
{
	return( m_State );
}
//--------------------------------------------------------------------------------
bool Texture3dConfigDX11::IsShaderResource() const
{
	return (m_State.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE;
}
//--------------------------------------------------------------------------------
bool Texture3dConfigDX11::IsRenderTarget() const
{
	return (m_State.BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::MakeShaderResource()
{
	m_State.BindFlags = D3D11_BIND_SHADER_RESOURCE;
}
//--------------------------------------------------------------------------------
void Texture3dConfigDX11::MakeRenderTarget()
{
	m_State.BindFlags = D3D11_BIND_RENDER_TARGET;
}
//--------------------------------------------------------------------------------
ShaderResourceViewConfigDX11* Texture3dConfigDX11::CreateSRV()
{
	ShaderResourceViewConfigDX11* pResult = new ShaderResourceViewConfigDX11;
	pResult->SetFormat(m_State.Format);
	D3D11_TEX3D_SRV srv;
	srv.MostDetailedMip = 0;
	srv.MipLevels = 1;
	pResult->SetTexture3D(srv);

	return pResult;
}
//--------------------------------------------------------------------------------
RenderTargetViewConfigDX11* Texture3dConfigDX11::CreateRTV()
{
	RenderTargetViewConfigDX11* pResult = new RenderTargetViewConfigDX11;
	pResult->SetFormat(m_State.Format);
	D3D11_TEX3D_RTV rtv;
	//TODO
	// not implement yet
	rtv.MipSlice = 0;
	pResult->SetTexture3D(rtv);

	return pResult;
}