//***************************************************************************************
// DeviceTextureDX12.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceTextureDX12.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"

using namespace forward;

DeviceTextureDX12::DeviceTextureDX12(FrameGraphTexture* tex)
	: DeviceResourceDX12(tex)
	, m_hCpuDescriptorHandle(CD3DX12_DEFAULT())
{

}