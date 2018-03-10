//***************************************************************************************
// DeviceTextureDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "DeviceTextureDX11.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"

using namespace forward;

DeviceTextureDX11::DeviceTextureDX11(FrameGraphTexture* tex)
	: DeviceResourceDX11(tex)
{}

ShaderResourceViewComPtr DeviceTextureDX11::GetSRView() const
{
	return m_srv;
}

UnorderedAccessViewComPtr DeviceTextureDX11::GetUAView() const
{
	return m_uav;
}
