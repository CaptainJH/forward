//***************************************************************************************
// DeviceDrawingStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceDrawingStateDX11.h"

using namespace forward;

DeviceDrawingStateDX11::DeviceDrawingStateDX11(forward::GraphicsObject* obj)
	: DeviceObject(obj)
{
}

DeviceObjComPtr DeviceDrawingStateDX11::GetDeviceObject()
{
	return m_deviceObjPtr;
}