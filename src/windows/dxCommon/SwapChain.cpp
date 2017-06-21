//--------------------------------------------------------------------------------
#include "SwapChain.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
SwapChain::SwapChain( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, Resource1Ptr resource )
{
	m_pSwapChain = pSwapChain;
	m_Resource = resource;
}
//--------------------------------------------------------------------------------
SwapChain::~SwapChain()
{
}
//--------------------------------------------------------------------------------
IDXGISwapChain* SwapChain::GetSwapChain()
{
	return( m_pSwapChain.Get() );
}
//--------------------------------------------------------------------------------
Resource1Ptr SwapChain::GetResourcePtr()
{
	return m_Resource;
}
//--------------------------------------------------------------------------------