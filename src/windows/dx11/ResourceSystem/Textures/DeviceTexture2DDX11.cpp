//***************************************************************************************
// DeviceTexture2DDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTexture2DDX11.h"

using namespace forward;

ResourceType DeviceTexture2DDX11::GetType()
{
	return RT_TEXTURE2D;
}

DepthStencilStateComPtr DeviceTexture2DDX11::GetDSView() const
{
	return m_dsv;
}

RenderTargetViewComPtr DeviceTexture2DDX11::GetRTView() const
{
	return m_rtv;
}