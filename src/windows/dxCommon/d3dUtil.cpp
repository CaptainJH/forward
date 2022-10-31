//***************************************************************************************
// d3dUtil.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include <dxgi.h>
#include "d3dUtil.h"

using namespace forward;

void forward::GetSurfaceInfo(u32 width, u32 height, DataFormatType df, u32* outNumBytes, u32* outRowBytes, u32* outNumRows)
{
	DXGI_FORMAT fmt = (DXGI_FORMAT)df;
	u32 numBytes = 0;
	u32 rowBytes = 0;
	u32 numRows = 0;

	bool bc = false;
	bool packed = false;
	bool planar = false;
	u32 bpe = 0;
	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bc = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		bc = true;
		bpe = 16;
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		packed = true;
		bpe = 4;
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		packed = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
		planar = true;
		bpe = 2;
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		planar = true;
		bpe = 4;
		break;
	}

	if (bc)
	{
		u32 numBlocksWide = 0;
		if (width > 0)
		{
			numBlocksWide = std::max<u32>(1, (width + 3) / 4);
		}
		u32 numBlocksHigh = 0;
		if (height > 0)
		{
			numBlocksHigh = std::max<u32>(1, (height + 3) / 4);
		}
		rowBytes = numBlocksWide * bpe;
		numRows = numBlocksHigh;
		numBytes = rowBytes * numBlocksHigh;
	}
	else if (packed)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numRows = height;
		numBytes = rowBytes * height;
	}
	else if (fmt == DXGI_FORMAT_NV11)
	{
		rowBytes = ((width + 3) >> 2) * 4;
		numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
		numBytes = rowBytes * numRows;
	}
	else if (planar)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
		numRows = height + ((height + 1) >> 1);
	}
	else
	{
		u32 bpp = DataFormat::GetNumBitsPerStruct(df);
		rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
		numRows = height;
		numBytes = rowBytes * height;
	}

	if (outNumBytes)
	{
		*outNumBytes = numBytes;
	}
	if (outRowBytes)
	{
		*outRowBytes = rowBytes;
	}
	if (outNumRows)
	{
		*outNumRows = numRows;
	}
}

D3D_PRIMITIVE_TOPOLOGY forward::Convert2D3DTopology(PrimitiveTopologyType topo)
{
	static D3D_PRIMITIVE_TOPOLOGY msTopology[] = 
	{
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
		D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST,
	};

	return msTopology[topo];
}