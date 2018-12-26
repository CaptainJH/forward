//--------------------------------------------------------------------------------
// SwapChainDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainDX11_h
#define SwapChainDX11_h
//--------------------------------------------------------------------------------
#include <vector>
#include <wrl.h>
#include <dxgi.h>
#include "render/ResourceSystem/DeviceResource.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChain
	{
	public:
		explicit SwapChain( Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, ResourcePtr resource );
		explicit SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain, shared_ptr<FrameGraphTexture2D> rt, shared_ptr<FrameGraphTexture2D> ds);
		virtual ~SwapChain();

		IDXGISwapChain* GetSwapChain();
		ResourcePtr GetResourcePtr();
		shared_ptr<FrameGraphTexture2D> GetCurrentRT() const;
		shared_ptr<FrameGraphTexture2D> GetCurrentDS() const;

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_pSwapChain;
		ResourcePtr							m_Resource;
		std::vector<shared_ptr<FrameGraphTexture2D>>			m_rts;
		shared_ptr<FrameGraphTexture2D>			m_ds;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainDX11_h
//--------------------------------------------------------------------------------

