//--------------------------------------------------------------------------------
// SwapChain
//
//--------------------------------------------------------------------------------
#pragma once
//--------------------------------------------------------------------------------
#include <vector>
#include <wrl.h>
#include <dxgi1_6.h>
#include "RHI/ResourceSystem/DeviceResource.h"
#include "RHI/ResourceSystem/Texture.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChain
	{
	public:
		// for DirectX12
		SwapChain(Microsoft::WRL::ComPtr< IDXGISwapChain1> pSwapChain, Vector<shared_ptr<Texture2D>> rt, shared_ptr<Texture2D> ds);
		~SwapChain();

		IDXGISwapChain* GetSwapChain();
		ResourcePtr GetResourcePtr();
		shared_ptr<Texture2D> GetCurrentRT() const;
		shared_ptr<Texture2D> GetCurrentDS() const;
		void Present(bool vsync=false) const;
		void PresentEnd() const;

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain3>	m_pSwapChain;
		ResourcePtr							m_Resource;
		std::vector<shared_ptr<Texture2D>>			m_rts;
		shared_ptr<Texture2D>			m_ds;

		mutable u32	m_currentBackBufferIndex = 0;
	};
};

