//***************************************************************************************
// dx12Util.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "dx12/dx12Util.h"

D3D12_SUBRESOURCE_DATA forward::ConvertSubResource(const Subresource* pData)
{
	assert(pData);

	D3D12_SUBRESOURCE_DATA data;
	data.pData = pData->data;
	data.RowPitch = pData->rowPitch;
	data.SlicePitch = pData->slicePitch;

	return data;
}