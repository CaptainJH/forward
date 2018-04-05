
//--------------------------------------------------------------------------------

#include "Texture1dDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Texture1dDX11::Texture1dDX11( Microsoft::WRL::ComPtr<ID3D11Texture1D> pTex )
{
	m_pTexture = pTex;

	ZeroMemory( &m_DesiredDesc, sizeof( D3D11_TEXTURE1D_DESC ) );
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_TEXTURE1D_DESC ) );
}
//--------------------------------------------------------------------------------
Texture1dDX11::~Texture1dDX11()
{
}
//--------------------------------------------------------------------------------
ResourceType Texture1dDX11::GetType()
{
	return( RT_TEXTURE1D );
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE1D_DESC Texture1dDX11::GetActualDescription()
{
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_TEXTURE1D_DESC ) );

	if ( m_pTexture )
		m_pTexture->GetDesc( &m_ActualDesc );

	return( m_ActualDesc );
}
//--------------------------------------------------------------------------------
D3D11_TEXTURE1D_DESC Texture1dDX11::GetDesiredDescription()
{
	return( m_DesiredDesc );
}
//--------------------------------------------------------------------------------
void Texture1dDX11::SetDesiredDescription( D3D11_TEXTURE1D_DESC description )
{
	m_DesiredDesc = description;
}
//--------------------------------------------------------------------------------
ID3D11Resource* Texture1dDX11::GetResource()
{
	return( m_pTexture.Get() );
}
//--------------------------------------------------------------------------------
u32 Texture1dDX11::GetEvictionPriority()
{
	u32 priority = 0;

	if ( m_pTexture )
		priority = m_pTexture->GetEvictionPriority();

	return( priority );
}
//--------------------------------------------------------------------------------
void Texture1dDX11::SetEvictionPriority( u32 EvictionPriority )
{
	if ( m_pTexture )
		m_pTexture->SetEvictionPriority( EvictionPriority );
}
//--------------------------------------------------------------------------------
void Texture1dDX11::CreateSRV()
{
	//TODO: not implement yet
}
//--------------------------------------------------------------------------------
ID3D11ShaderResourceView* Texture1dDX11::GetSRV() const
{
	//TODO: not implement yet
	return nullptr;
}