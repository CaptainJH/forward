//--------------------------------------------------------------------------------
// SwapChainDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainDX11_h
#define SwapChainDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <dxgi.h>
#include "render/ResourceSystem/DeviceResource.h"
#include "FrameGraph/FrameGraphObject.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChain
	{
	public:
		explicit SwapChain( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, ResourcePtr resource );
		explicit SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, shared_ptr<FrameGraphObject> rt, shared_ptr<FrameGraphObject> ds);
		virtual ~SwapChain();

		IDXGISwapChain* GetSwapChain();
		ResourcePtr GetResourcePtr();

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_pSwapChain;
		ResourcePtr							m_Resource;
		shared_ptr<FrameGraphObject>			m_rt;
		shared_ptr<FrameGraphObject>			m_ds;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainDX11_h
//--------------------------------------------------------------------------------

