//***************************************************************************************
// dx11Util.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "dx11_Hieroglyph/dx11Util.h"

D3D11_SUBRESOURCE_DATA forward::ConvertSubResource(const Subresource* pData)
{
	assert(pData);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = pData->data;
	data.SysMemPitch = pData->rowPitch;
	data.SysMemSlicePitch = pData->slicePitch;

	return data;
}