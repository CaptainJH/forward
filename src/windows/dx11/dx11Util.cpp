//***************************************************************************************
// dx11Util.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "dx11/dx11Util.h"

using namespace forward;

EResult forward::FillInitDataDX11(u32 width, u32 height, u32 depth, u32 mipCount, u32 arraySize, DataFormatType format,
	u32 maxSize, u32 bitSize, const u8* bitData, u32& twidth, u32& theight, u32& tdepth, u32& skipMip, D3D11_SUBRESOURCE_DATA* initData)
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
		for (u32 i = 0; i < mipCount; ++j)
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
				initData[index].pSysMem = (const void*)pSrcBits;
				initData[index].SysMemPitch = static_cast<UINT>(RowBytes);
				initData[index].SysMemSlicePitch = static_cast<UINT>(NumBytes);
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