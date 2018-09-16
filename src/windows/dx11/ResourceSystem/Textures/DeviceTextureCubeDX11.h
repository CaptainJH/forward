//***************************************************************************************
// DeviceTextureCubeDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "DeviceTextureDX11.h"
#include "dx11/dx11Util.h"

namespace forward
{
	class FrameGraphTextureCube;

	class DeviceTextureCubeDX11 : public DeviceTextureDX11
	{
	public:
		DeviceTextureCubeDX11(ID3D11Device* device, FrameGraphTextureCube* tex);
		//DeviceTextureCubeDX11(ID3D11Texture2D* deviceTex, FrameGraphTexture2D* tex);

		ID3D11Texture2D* GetDXTexture2DPtr();
		shared_ptr<FrameGraphTextureCube> GetFrameGraphTextureCube();

		void					SyncCPUToGPU() override;
		//void					SyncGPUToCPU(ID3D11DeviceContext* context);

	private:
		void CreateSRView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		void CreateUAView(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
		//void CreateStaging(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& tx);
	};
}