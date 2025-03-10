//***************************************************************************************
// dx12Util.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "windows/dx12/dx12Util.h"

#pragma warning(disable : 4238)

using namespace forward;

//D3D12_SUBRESOURCE_DATA forward::ConvertSubResource(const Subresource* pData)
//{
//	assert(pData);
//
//	D3D12_SUBRESOURCE_DATA data;
//	data.pData = pData->data;
//	data.RowPitch = pData->rowPitch;
//	data.SlicePitch = pData->slicePitch;
//
//	return data;
//}

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

EResult forward::FillInitDataDX12(u32 width, u32 height, u32 depth, u32 mipCount, u32 arraySize, DataFormatType format,
	u32 maxSize, u32 bitSize, const u8* bitData, u32& twidth, u32& theight, u32& tdepth, u32& skipMip, D3D12_SUBRESOURCE_DATA* initData)
{
	if (!bitData || !initData)
	{
		return E_RESULT_NULL_POINTER_ARGUMENT;
	}

	skipMip = 0;
	twidth = 0;
	theight = 0;
	tdepth = 0;

	u32 NumBytes = 0;
	u32 RowBytes = 0;
	const u8* pSrcBits = bitData;
	const u8* pEndBits = bitData + bitSize;

	u32 index = 0;
	for (u32 j = 0; j < arraySize; ++j)
	{
		u32 w = width;
		u32 h = height;
		u32 d = depth;
		for (u32 i = 0; i < mipCount; ++i)
		{
			GetSurfaceInfo(w,
				h,
				format,
				&NumBytes,
				&RowBytes,
				nullptr
			);

			if ((mipCount <= 1) || !maxSize || (w <= maxSize && h <= maxSize && d <= maxSize))
			{
				if (!twidth)
				{
					twidth = w;
					theight = h;
					tdepth = d;
				}

				assert(index < mipCount * arraySize);
				_Analysis_assume_(index < mipCount * arraySize);
				initData[index].pData = (const void*)pSrcBits;
				initData[index].RowPitch = static_cast<UINT>(RowBytes);
				initData[index].SlicePitch = static_cast<UINT>(NumBytes);
				++index;
			}
			else if (!j)
			{
				// Count number of skipped mipmaps (first item only)
				++skipMip;
			}

			if (pSrcBits + (NumBytes*d) > pEndBits)
			{
				return E_RESULT_CORRUPT_DATA_SOURCE;
			}

			pSrcBits += NumBytes * d;

			w = w >> 1;
			h = h >> 1;
			d = d >> 1;
			if (w == 0)
			{
				w = 1;
			}
			if (h == 0)
			{
				h = 1;
			}
			if (d == 0)
			{
				d = 1;
			}
		}
	}

	return (index > 0) ? E_RESULT_NO_ERROR : E_RESULT_UNKNOWN_ERROR;
}

DeviceResCom12Ptr forward::CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	DeviceResCom12Ptr& uploadBuffer)
{
	DeviceResCom12Ptr defaultBuffer;

	// Create the actual default buffer resource.
	D3D12_HEAP_PROPERTIES heapPropDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	HR(device->CreateCommittedResource(
		&heapPropDefault,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	D3D12_HEAP_PROPERTIES heapPropUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	HR(device->CreateCommittedResource(
		&heapPropUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;


	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	D3D12_RESOURCE_BARRIER barrier0 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &barrier0);
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	D3D12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &barrier1);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}