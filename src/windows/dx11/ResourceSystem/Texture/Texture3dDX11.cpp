//--------------------------------------------------------------------------------
#include "Texture3dDX11.h"
#include "windows/dx11/RendererDX11.h"
#include "windows/dx11/ResourceSystem/ResourceView/ShaderResourceViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Texture3dDX11::Texture3dDX11( Microsoft::WRL::ComPtr<ID3D11Texture3D> pTex )
{
	m_pTexture = pTex;

	ZeroMemory( &m_DesiredDesc, sizeof( D3D11_TEXTURE3D_DESC ) );
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_TEXTURE3D_DESC ) );
}
//--------------------------------------------------------------------------------
Texture3dDX11::Texture3dDX11(Microsoft::WRL::ComPtr<ID3D11Texture3D> pTex, Texture3dConfig* pConfig)
{
	m_pResourceConfig = new Texture3dConfig(*pConfig);
	m_pTexture = pTex;

	ZeroMemory(&m_DesiredDesc, sizeof(D3D11_TEXTURE3D_DESC));
	ZeroMemory(&m_ActualDesc, sizeof(D3D11_TEXTURE3D_DESC));

	Texture3dConfigDX11 configDX11 = *pConfig;
	D3D11_TEXTURE3D_DESC desc = configDX11.GetTextureDesc();
	ShaderResourceViewConfigDX11* pSRVConfig = nullptr;

	if (configDX11.IsShaderResource())
	{
		pSRVConfig = configDX11.CreateSRV();
	}

	CreateSRV(desc.BindFlags, pSRVConfig);
}
//--------------------------------------------------------------------------------
Texture3dDX11::~Texture3dDX11()
{
}
//--------------------------------------------------------------------------------
ResourceType Texture3dDX11::GetType()
{
	return( RT_TEXTURE3D );
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE3D_DESC Texture3dDX11::GetActualDescription()
{
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_TEXTURE3D_DESC ) );

	if ( m_pTexture )
		m_pTexture->GetDesc( &m_ActualDesc );

	return( m_ActualDesc );
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE3D_DESC Texture3dDX11::GetDesiredDescription()
{
	return( m_DesiredDesc );
}
//--------------------------------------------------------------------------------
void Texture3dDX11::SetDesiredDescription( D3D11_TEXTURE3D_DESC description )
{
	m_DesiredDesc = description;
}
//--------------------------------------------------------------------------------
ID3D11Resource* Texture3dDX11::GetResource()
{
	return( m_pTexture.Get() );
}
//--------------------------------------------------------------------------------
u32 Texture3dDX11::GetEvictionPriority()
{
	u32 priority = 0;

	if ( m_pTexture )
		priority = m_pTexture->GetEvictionPriority();

	return( priority );
}
//--------------------------------------------------------------------------------
void Texture3dDX11::SetEvictionPriority( u32 EvictionPriority )
{
	if ( m_pTexture )
		m_pTexture->SetEvictionPriority( EvictionPriority );
}
//--------------------------------------------------------------------------------
ID3D11ShaderResourceView* Texture3dDX11::GetSRV() const
{
	if (m_iResourceSRV < 0)
		return nullptr;

	auto srv = RendererDX11::Get()->GetShaderResourceViewByIndex(m_iResourceSRV);
	return srv.GetSRV();
}
//--------------------------------------------------------------------------------
void Texture3dDX11::CreateSRV(u32 BindFlags, ShaderResourceViewConfigDX11* pSRVConfig)
{
	RendererDX11* pRendererDX11 = RendererDX11::Get();

	// Depending on the bind flags used to create the resource, we create a set
	// of default views to be used.

	if ((BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc = pSRVConfig ? &pSRVConfig->GetSRVDesc() : nullptr;
		m_iResourceSRV = pRendererDX11->CreateShaderResourceView(this, pDesc);
	}
}