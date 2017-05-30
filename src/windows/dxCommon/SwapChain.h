//--------------------------------------------------------------------------------
// SwapChainDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainDX11_h
#define SwapChainDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <dxgi.h>
#include "render/ResourceSystem/ResourceProxy.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChain
	{
	public:
		explicit SwapChain( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, ResourcePtrBase resource );
		virtual ~SwapChain();

		IDXGISwapChain* GetSwapChain();
		ResourcePtrBase GetResourcePtr();

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_pSwapChain;
		ResourcePtrBase							m_Resource;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainDX11_h
//--------------------------------------------------------------------------------

