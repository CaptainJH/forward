//--------------------------------------------------------------------------------
#include "RenderTargetViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
RenderTargetViewDX11::RenderTargetViewDX11( RenderTargetViewComPtr pView )
{
	m_pRenderTargetView = pView;
}
//--------------------------------------------------------------------------------
RenderTargetViewDX11::~RenderTargetViewDX11()
{
}
//--------------------------------------------------------------------------------
ID3D11RenderTargetView* RenderTargetViewDX11::GetRTV()
{
	return( m_pRenderTargetView.Get() );
}
//--------------------------------------------------------------------------------
