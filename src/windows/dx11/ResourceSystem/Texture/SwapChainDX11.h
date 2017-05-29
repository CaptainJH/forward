//--------------------------------------------------------------------------------
// SwapChainDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainDX11_h
#define SwapChainDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <dxgi.h>
#include "dx11/ResourceSystem/ResourceProxyDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChainDX11
	{
	public:
		explicit SwapChainDX11( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, ResourcePtr resource );
		virtual ~SwapChainDX11();

		IDXGISwapChain* GetSwapChain();
		ResourcePtr GetResourcePtr();

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_pSwapChain;
		ResourcePtr								m_Resource;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainDX11_h
//--------------------------------------------------------------------------------

