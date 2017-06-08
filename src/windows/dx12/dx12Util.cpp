//***************************************************************************************
// dx12Util.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "dx12/dx12Util.h"

using namespace forward;

D3D12_SUBRESOURCE_DATA forward::ConvertSubResource(const Subresource* pData)
{
	assert(pData);

	D3D12_SUBRESOURCE_DATA data;
	data.pData = pData->data;
	data.RowPitch = pData->rowPitch;
	data.SlicePitch = pData->slicePitch;

	return data;
}

u32 forward::CalcConstantBufferByteSize(u32 byteSize)
{
	// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
	return (byteSize + 255) & ~255;
}