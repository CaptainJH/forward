//***************************************************************************************
// DeviceBlendStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceBlendStateDX11.h"

using namespace forward;

ID3D11BlendState* DeviceBlendStateDX11::GetBlendState()
{
	return static_cast<ID3D11BlendState*>(m_deviceObjPtr.Get());
}