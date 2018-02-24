//--------------------------------------------------------------------------------
#include "ShaderResourceViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ShaderResourceViewDX11::ShaderResourceViewDX11( ShaderResourceViewComPtr pView )
{
	m_pShaderResourceView = pView;
}
//--------------------------------------------------------------------------------
ShaderResourceViewDX11::~ShaderResourceViewDX11()
{
}
//--------------------------------------------------------------------------------
ID3D11ShaderResourceView* ShaderResourceViewDX11::GetSRV()
{
	return( m_pShaderResourceView.Get() );
}
//--------------------------------------------------------------------------------
