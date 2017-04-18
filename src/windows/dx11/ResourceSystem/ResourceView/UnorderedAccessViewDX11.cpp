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
