//***************************************************************************************
// dx11Util.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "dxCommon/d3dUtil.h"

typedef Microsoft::WRL::ComPtr<ID3D12Device> DeviceComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueueComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocatorComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandListComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12Fence> FenceComPtr;

typedef Microsoft::WRL::ComPtr<ID3D12Resource> ResourceComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12Buffer> BufferComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12Texture1D> Texture1DComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12Texture2D> Texture2DComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12Texture3D> Texture3DComPtr;

typedef Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12ShaderResourceView> ShaderResourceViewComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12DepthStencilView> DepthStencilViewComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12RenderTargetView> RenderTargetViewComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12UnorderedAccessView> UnorderedAccessViewComPtr;
//									
//typedef Microsoft::WRL::ComPtr<ID3D12BlendState> BlendStateComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12DepthStencilState> DepthStencilStateComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12RasterizerState> RasterizerStateComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12SamplerState> SamplerStateComPtr;
//typedef Microsoft::WRL::ComPtr<ID3D12InputLayout> InputLayoutComPtr;

namespace forward
{
	D3D12_SUBRESOURCE_DATA ConvertSubResource(const Subresource* pData);
}