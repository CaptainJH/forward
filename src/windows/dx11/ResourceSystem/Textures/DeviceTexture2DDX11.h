//***************************************************************************************
// DeviceTexture2DDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/Texture.h"
#include "DeviceTextureDX11.h"
#include "dx11/dx11Util.h"

namespace forward
{
	class Texture2D;

	class DeviceTexture2DDX11 : public DeviceTextureDX11
	{
	public:

		static DeviceTexture2DDX11* BuildDeviceTexture2DDX11(const std::string& name, ID3D11Texture2D* tex, ResourceUsage usage=RU_IMMUTABLE);

		DeviceTexture2DDX11(ID3D11Device* device, Texture2D* tex);
		DeviceTexture2DDX11(ID3D11Texture2D* deviceTex, Texture2D* tex);

		ID3D11Texture2D* GetDXTexture2DPtr();
		shared_ptr<Texture2D> GetFrameGraphTexture2D();

		DepthStencilViewComPtr	GetDSView() const;
		RenderTargetViewComPtr	GetRTView() const;

		void					SyncCPUToGPU() override;
		void					SyncGPUToCPU(ID3D11DeviceContext* context);

	private:
		void CreateSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateUAView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateDSView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateDSSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateRTView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateStaging(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);

		static DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthFormat);
		static DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthFormat);

	protected:
		DepthStencilViewComPtr		m_dsv;
		RenderTargetViewComPtr		m_rtv;
	};
}