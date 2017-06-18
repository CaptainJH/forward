//--------------------------------------------------------------------------------
#include "Texture2dDX11.h"
#include "windows/dx11/RendererDX11.h"
#include "windows/dx11/ResourceSystem/ResourceView/ShaderResourceViewDX11.h"
#include "windows/dx11/ResourceSystem/ResourceView/RenderTargetViewDX11.h"
#include "windows/dx11/ResourceSystem/ResourceView/DepthStencilViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Texture2dDX11::Texture2dDX11( Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex )
{
	m_pTexture = pTex;

	ZeroMemory( &m_DesiredDesc, sizeof( D3D11_TEXTURE2D_DESC ) );
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_TEXTURE2D_DESC ) );
}
//--------------------------------------------------------------------------------
Texture2dDX11::Texture2dDX11(Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex, Texture2dConfig* pConfig)
{
	m_pResourceConfig = pConfig;
	m_pTexture = pTex;

	ZeroMemory(&m_DesiredDesc, sizeof(D3D11_TEXTURE2D_DESC));
	ZeroMemory(&m_ActualDesc, sizeof(D3D11_TEXTURE2D_DESC));

	Texture2dConfigDX11 configDX11 = *pConfig;
	D3D11_TEXTURE2D_DESC desc = configDX11.GetTextureDesc();
	ShaderResourceViewConfigDX11* pSRVConfig = nullptr;
	RenderTargetViewConfigDX11* pRTVConfig = nullptr;
	UnorderedAccessViewConfigDX11* pUAVConfig = nullptr;
	DepthStencilViewConfigDX11* pDSVConfig = nullptr;

	if (configDX11.IsShaderResource())
	{
		pSRVConfig = configDX11.CreateSRV();
	}
	if (configDX11.IsDepthStencil())
	{
		pDSVConfig = configDX11.CreateDSV();

		if (configDX11.IsShaderResource())
		{
			DXGI_FORMAT format = static_cast<DXGI_FORMAT>(Texture2dConfigDX11::GetDepthSRVFormat(
				static_cast<DataFormatType>(pDSVConfig->GetFormat())
			));
			pSRVConfig->SetFormat(format);
		}
	}

	CreateResourceViews(desc.BindFlags, pSRVConfig, pRTVConfig, pUAVConfig, pDSVConfig);
}
//--------------------------------------------------------------------------------
Texture2dDX11::~Texture2dDX11()
{
}
//--------------------------------------------------------------------------------
ResourceType Texture2dDX11::GetType()
{
	return( RT_TEXTURE2D );
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE2D_DESC Texture2dDX11::GetActualDescription()
{
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_TEXTURE2D_DESC ) );

	if ( m_pTexture )
		m_pTexture->GetDesc( &m_ActualDesc );

	return( m_ActualDesc );
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE2D_DESC Texture2dDX11::GetDesiredDescription()
{
	return( m_DesiredDesc );
}
//--------------------------------------------------------------------------------
void Texture2dDX11::SetDesiredDescription( D3D11_TEXTURE2D_DESC description )
{
	m_DesiredDesc = description;
}
//--------------------------------------------------------------------------------
ID3D11Resource* Texture2dDX11::GetResource()
{
	return( m_pTexture.Get() );
}
//--------------------------------------------------------------------------------
u32 Texture2dDX11::GetEvictionPriority()
{
	u32 priority = 0;

	if ( m_pTexture )
		priority = m_pTexture->GetEvictionPriority();

	return( priority );
}
//--------------------------------------------------------------------------------
void Texture2dDX11::SetEvictionPriority( u32 EvictionPriority )
{
	if ( m_pTexture )
		m_pTexture->SetEvictionPriority( EvictionPriority );
}
//--------------------------------------------------------------------------------
ID3D11ShaderResourceView* Texture2dDX11::GetSRV() const
{
	if (m_iResourceSRV < 0)
		return nullptr;

	auto srv = RendererDX11::Get()->GetShaderResourceViewByIndex(m_iResourceSRV);
	return srv.GetSRV();
}
//--------------------------------------------------------------------------------
ID3D11RenderTargetView* Texture2dDX11::GetRTV() const
{
	if (m_iResourceRTV < 0)
		return nullptr;

	auto rtv = RendererDX11::Get()->GetRenderTargetViewByIndex(m_iResourceRTV);
	return rtv.GetRTV();
}
//--------------------------------------------------------------------------------
ID3D11DepthStencilView* Texture2dDX11::GetDSV() const
{
	if (m_iResourceDSV < 0)
		return nullptr;

	auto dsv = RendererDX11::Get()->GetDepthStencilViewByIndex(m_iResourceDSV);
	return dsv.GetDSV();
}
//--------------------------------------------------------------------------------
void Texture2dDX11::CreateResourceViews(u32 BindFlags,
	ShaderResourceViewConfigDX11* pSRVConfig,
	RenderTargetViewConfigDX11* pRTVConfig,
	UnorderedAccessViewConfigDX11* pUAVConfig,
	DepthStencilViewConfigDX11* pDSVConfig)
{

	RendererDX11* pRendererDX11 = RendererDX11::Get();

	// Depending on the bind flags used to create the resource, we create a set
	// of default views to be used.

	if ((BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc = pSRVConfig ? &pSRVConfig->GetSRVDesc() : nullptr;
		m_iResourceSRV = pRendererDX11->CreateShaderResourceView(m_ResourceID, pDesc);
	}

	if ((BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC* pDesc = pRTVConfig ? &pRTVConfig->GetRTVDesc() : nullptr;
		m_iResourceRTV = pRendererDX11->CreateRenderTargetView(m_ResourceID, pDesc);
	}

	if ((BindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = pDSVConfig ? &pDSVConfig->GetDSVDesc() : nullptr;
		m_iResourceDSV = pRendererDX11->CreateDepthStencilView(m_ResourceID, pDesc);
	}

	if ((BindFlags & D3D11_BIND_UNORDERED_ACCESS) == D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc = pUAVConfig ? &pUAVConfig->GetUAVDesc() : nullptr;
		m_iResourceUAV = pRendererDX11->CreateUnorderedAccessView(m_ResourceID, pDesc);
	}
}