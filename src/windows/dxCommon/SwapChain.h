//--------------------------------------------------------------------------------
// SwapChainDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainDX11_h
#define SwapChainDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <dxgi.h>
#include "render/ResourceSystem/Resource.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChain
	{
	public:
		explicit SwapChain( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, Resource1Ptr resource );
		virtual ~SwapChain();

		IDXGISwapChain* GetSwapChain();
		Resource1Ptr GetResourcePtr();

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_pSwapChain;
		Resource1Ptr							m_Resource;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainDX11_h
//--------------------------------------------------------------------------------

