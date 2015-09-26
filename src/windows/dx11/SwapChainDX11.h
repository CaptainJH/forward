//--------------------------------------------------------------------------------
// SwapChainDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainDX11_h
#define SwapChainDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChainDX11
	{
	public:
		//TODO: ResourcePtr
		//explicit SwapChainDX11( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, ResourcePtr resource );
		virtual ~SwapChainDX11();

		IDXGISwapChain* GetSwapChain();

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_pSwapChain;
		//TODO: ResourcePtr
		//ResourcePtr								m_Resource;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainDX11_h
//--------------------------------------------------------------------------------

