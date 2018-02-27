//***************************************************************************************
// dx11Util.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <d3d11_2.h>
#include <wrl.h>
#include "dxCommon/d3dUtil.h"

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

typedef Microsoft::WRL::ComPtr<ID3D11BlendState> BlendStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayoutComPtr;

namespace forward
{
	D3D11_SUBRESOURCE_DATA ConvertSubResource(const Subresource* pData);
}