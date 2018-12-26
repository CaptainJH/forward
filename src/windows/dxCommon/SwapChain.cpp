//--------------------------------------------------------------------------------
#include "SwapChain.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
SwapChain::SwapChain( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, ResourcePtr resource )
{
	m_pSwapChain = pSwapChain;
	m_Resource = resource;
}

SwapChain::SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, shared_ptr<FrameGraphTexture2D> rt, shared_ptr<FrameGraphTexture2D> ds)
{
	m_pSwapChain = pSwapChain;
	m_rts.push_back(rt);
	m_ds = ds;
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
ResourcePtr SwapChain::GetResourcePtr()
{
	return m_Resource;
}
//--------------------------------------------------------------------------------
shared_ptr<FrameGraphTexture2D> SwapChain::GetCurrentRT() const
{
	assert(!m_rts.empty());
	return m_rts.front();
}
//--------------------------------------------------------------------------------
shared_ptr<FrameGraphTexture2D> SwapChain::GetCurrentDS() const
{
	return m_ds;
}