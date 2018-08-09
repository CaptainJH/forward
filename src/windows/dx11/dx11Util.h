//***************************************************************************************
// dx11Util.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <d3d11_2.h>
#include <wrl.h>
#include "utilities/Utils.h"
#include "dxCommon/d3dUtil.h"
#include "utilities/Log.h"

typedef Microsoft::WRL::ComPtr<ID3D11Device> DeviceComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContextComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11Query> QueryComPtr;

typedef Microsoft::WRL::ComPtr<ID3D11DeviceChild> DeviceObjComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11Resource> DeviceResComPtr;

typedef Microsoft::WRL::ComPtr<ID3D11Buffer> BufferComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11Texture1D> Texture1DComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2DComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11Texture3D> Texture3DComPtr;

typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceViewComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilViewComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetViewComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UnorderedAccessViewComPtr;

typedef Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShaderComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShaderComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11GeometryShader> GeometryShaderComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShaderComPtr;

typedef Microsoft::WRL::ComPtr<ID3D11BlendState> BlendStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayoutComPtr;

typedef Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> UserDefinedAnnotationComPtr;

#define D3D11_RESOURCE_MISC_NONE	0
#define D3D11_CPU_ACCESS_NONE		0
#define D3D11_BIND_NONE				0

namespace forward
{
	EResult FillInitDataDX11(u32 width, u32 height, u32 depth, u32 mipCount, u32 arraySize, DataFormatType format,
		u32 maxSize, u32 bitSize, const u8* bitData, u32& twidth, u32& theight, u32& tdepth, u32& skipMip, D3D11_SUBRESOURCE_DATA* initData);
}