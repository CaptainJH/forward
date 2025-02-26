//--------------------------------------------------------------------------------
#include <SDL3/SDL_Render.h>
#include "SwapChain.h"
#include "d3dUtil.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
SwapChain::SwapChain(Microsoft::WRL::ComPtr< IDXGISwapChain1> pSwapChain, Vector<shared_ptr<Texture2D>> rt, shared_ptr<Texture2D> ds, SDL_Renderer* r)
	: m_sdlRenderer(r) {
	HR(pSwapChain.As(&m_pSwapChain));
	updateBackBufferIndex();
	m_rts = rt;
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
shared_ptr<Texture2D> SwapChain::GetCurrentRT() const
{
	assert(!m_rts.empty());
	auto ret = m_rts[m_currentBackBufferIndex];
	return ret;
}
//--------------------------------------------------------------------------------
shared_ptr<Texture2D> SwapChain::GetCurrentDS() const
{
	return m_ds;
}
//--------------------------------------------------------------------------------
void SwapChain::Present(bool vsync) const
{
	assert(!m_rts.empty());
	if (m_sdlRenderer)
		SDL_RenderPresent(m_sdlRenderer);
	else
		HR(m_pSwapChain->Present(0, vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING));
	updateBackBufferIndex();
}

void SwapChain::updateBackBufferIndex() const
{
	m_currentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}