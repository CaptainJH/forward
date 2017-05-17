//--------------------------------------------------------------------------------
#include "UnorderedAccessViewDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
UnorderedAccessViewDX11::UnorderedAccessViewDX11( UnorderedAccessViewComPtr pView )
{
	m_pUnorderedAccessView = pView;
}
//--------------------------------------------------------------------------------
UnorderedAccessViewDX11::~UnorderedAccessViewDX11()
{
}
//--------------------------------------------------------------------------------
ID3D11UnorderedAccessView* UnorderedAccessViewDX11::GetUAV()
{
	return m_pUnorderedAccessView.Get();
}