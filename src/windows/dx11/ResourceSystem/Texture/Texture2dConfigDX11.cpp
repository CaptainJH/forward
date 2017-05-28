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
void Texture2dConfigDX11::SetDepthBuffer(u32 width, u32 height)
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
void Texture2dConfigDX11::SetColorBuffer(u32 width, u32 height)
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
void Texture2dConfigDX11::SetWidth( u32 state )
{
	m_State.Width = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetHeight( u32 state )
{
	m_State.Height = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetMipLevels( u32 state )
{
	m_State.MipLevels = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetArraySize( u32 state )
{
	m_State.ArraySize = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetFormat( DXGI_FORMAT state )
{
	m_State.Format = state;
}
//--------------------------------------------------------------------------------
DXGI_FORMAT Texture2dConfigDX11::GetFormat() const
{
	return m_State.Format;
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
void Texture2dConfigDX11::SetBindFlags( u32 state )
{
	m_State.BindFlags = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetCPUAccessFlags( u32 state )
{
	m_State.CPUAccessFlags = state;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::SetMiscFlags( u32 state )
{
	m_State.MiscFlags = state;
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE2D_DESC Texture2dConfigDX11::GetTextureDesc()
{
	return( m_State );
}
//--------------------------------------------------------------------------------
bool Texture2dConfigDX11::IsShaderResource() const
{
	return (m_State.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE;
}
//--------------------------------------------------------------------------------
bool Texture2dConfigDX11::IsRenderTarget() const
{
	return (m_State.BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET;
}
//--------------------------------------------------------------------------------
bool Texture2dConfigDX11::IsDepthStencil() const
{
	return (m_State.BindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL;
}
//--------------------------------------------------------------------------------
bool Texture2dConfigDX11::IsTextureArray() const
{
	return m_State.ArraySize > 1;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::MakeShaderResource()
{
	m_State.BindFlags = D3D11_BIND_SHADER_RESOURCE;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::MakeRenderTarget()
{
	m_State.BindFlags = D3D11_BIND_RENDER_TARGET;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::MakeDepthStencil()
{
	m_State.BindFlags = D3D11_BIND_DEPTH_STENCIL;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::MakeRenderTargetAndShaderResource()
{
	m_State.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
}
//--------------------------------------------------------------------------------
void Texture2dConfigDX11::MakeDepthStencilAndShaderResource()
{
	m_State.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
}
//--------------------------------------------------------------------------------
ShaderResourceViewConfigDX11* Texture2dConfigDX11::CreateSRV()
{
	ShaderResourceViewConfigDX11* pResult = new ShaderResourceViewConfigDX11;
	pResult->SetFormat(m_State.Format);
	if (IsTextureArray())
	{
		D3D11_TEX2D_ARRAY_SRV srv;
		srv.ArraySize = m_State.ArraySize;
		srv.FirstArraySlice = 0;
		srv.MipLevels = 1;
		srv.MostDetailedMip = 0;
		pResult->SetTexture2DArray(srv);
	}
	else
	{
		D3D11_TEX2D_SRV srv;
		srv.MostDetailedMip = 0;
		srv.MipLevels = 1;
		pResult->SetTexture2D(srv);
	}

	return pResult;
}
//--------------------------------------------------------------------------------
DepthStencilViewConfigDX11* Texture2dConfigDX11::CreateDSV()
{
	DepthStencilViewConfigDX11* pResult = new DepthStencilViewConfigDX11;
	pResult->SetFormat(m_State.Format);
	if (IsTextureArray())
	{
		D3D11_TEX2D_ARRAY_DSV dsv;
		dsv.ArraySize = m_State.ArraySize;
		dsv.FirstArraySlice = 0;
		dsv.MipSlice = 0;
		pResult->SetTexture2DArray(dsv);
	}
	else if (m_State.SampleDesc.Count > 1)
	{
		D3D11_TEX2DMS_DSV dsv;
		dsv.UnusedField_NothingToDefine = 0;
		pResult->SetTexture2DMS(dsv);
	}
	else
	{
		D3D11_TEX2D_DSV dsv;
		dsv.MipSlice = 0;
		pResult->SetTexture2D(dsv);
	}


	return pResult;
}
//--------------------------------------------------------------------------------
DXGI_FORMAT Texture2dConfigDX11::GetDepthResourceFormat(DXGI_FORMAT depthFormat)
{
	if (depthFormat == DXGI_FORMAT_D16_UNORM)
	{
		return DXGI_FORMAT_R16_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		return DXGI_FORMAT_R24G8_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT)
	{
		return DXGI_FORMAT_R32_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
	{
		return DXGI_FORMAT_R32G8X24_TYPELESS;
	}

	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT Texture2dConfigDX11::GetDepthSRVFormat(DXGI_FORMAT depthFormat)
{
	if (depthFormat == DXGI_FORMAT_D16_UNORM)
	{
		return DXGI_FORMAT_R16_UNORM;
	}

	if (depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT)
	{
		return DXGI_FORMAT_R32_FLOAT;
	}

	if (depthFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
	{
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}

	return DXGI_FORMAT_UNKNOWN;
}