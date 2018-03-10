//***************************************************************************************
// DeviceTexture2DDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceTextureDX11.h"
#include "dx11/dx11Util.h"

namespace forward
{
	class FrameGraphTexture2D;

	class DeviceTexture2DDX11 : public DeviceTextureDX11
	{
	public:

		DeviceTexture2DDX11(ID3D11Device* device, FrameGraphTexture2D* tex);

		ResourceType	GetType() override;
		ID3D11Texture2D* GetDXTexture2DPtr();
		FrameGraphTexture2D* GetFrameGraphTexture2D();

		DepthStencilViewComPtr		GetDSView() const;
		RenderTargetViewComPtr		GetRTView() const;

	private:
		void CreateSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateUAView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateDSView(ID3D11Device* device);
		void CreateDSSRView(ID3D11Device* device);
		void CreateRTView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);

		static DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthFormat);
		static DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthFormat);

	protected:
		DepthStencilViewComPtr		m_dsv;
		RenderTargetViewComPtr		m_rtv;

	};
}