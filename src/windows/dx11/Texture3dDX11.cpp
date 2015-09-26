//--------------------------------------------------------------------------------
#include "Texture3dDX11.h"
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
forward::UINT Texture3dDX11::GetEvictionPriority()
{
	forward::UINT priority = 0;

	if ( m_pTexture )
		priority = m_pTexture->GetEvictionPriority();

	return( priority );
}
//--------------------------------------------------------------------------------
void Texture3dDX11::SetEvictionPriority( forward::UINT EvictionPriority )
{
	if ( m_pTexture )
		m_pTexture->SetEvictionPriority( EvictionPriority );
}
//--------------------------------------------------------------------------------