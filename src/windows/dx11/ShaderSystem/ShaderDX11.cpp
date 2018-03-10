#include "ShaderDX11.h"

using namespace forward;

ShaderDX11::ShaderDX11(forward::FrameGraphObject* obj)
	: ShaderDX(obj)
{}

ShaderDX11::~ShaderDX11()
{}

DeviceObjComPtr ShaderDX11::GetDeviceObject()
{
	return m_deviceObjPtr;
}