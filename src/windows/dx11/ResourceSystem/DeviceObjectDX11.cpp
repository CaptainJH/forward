
//--------------------------------------------------------------------------------
#include "DeviceObjectDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
DeviceObjectDX11::DeviceObjectDX11()
{
}
//--------------------------------------------------------------------------------
DeviceObjectDX11::~DeviceObjectDX11()
{
}

DeviceObjComPtr DeviceObjectDX11::GetDeviceObject()
{
	return m_deviceObjPtr;
}