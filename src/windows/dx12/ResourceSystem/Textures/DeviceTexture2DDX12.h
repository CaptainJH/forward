//***************************************************************************************
// DeviceTexture2DDX12.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "DeviceTextureDX12.h"

namespace forward
{
	class FrameGraphTexture2D;

	class DeviceTexture2DDX12 : public DeviceTextureDX12
	{
	public:
		DeviceTexture2DDX12(ID3D12Device* device, FrameGraphTexture2D* tex);
		//DeviceTexture2DDX12(ID3D11Texture2D* deviceTex, FrameGraphTexture2D* tex);

	protected:
	};
}