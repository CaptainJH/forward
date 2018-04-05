//--------------------------------------------------------------------------------

#include "DepthStencilViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
DepthStencilViewDX11::DepthStencilViewDX11( DepthStencilViewComPtr pView )
{
	m_pDepthStencilView = pView;
}
//--------------------------------------------------------------------------------
DepthStencilViewDX11::~DepthStencilViewDX11()
{
}
//--------------------------------------------------------------------------------
ID3D11DepthStencilView* DepthStencilViewDX11::GetDSV()
{
	return m_pDepthStencilView.Get();
}