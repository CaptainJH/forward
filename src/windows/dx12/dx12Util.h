//***************************************************************************************
// dx11Util.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "dx12/d3dx12.h"
#include "dxCommon/d3dUtil.h"
#include "utilities/Log.h"
#include "utilities/Utils.h"
#include <unordered_map>

typedef Microsoft::WRL::ComPtr<ID3D12Device5> DeviceCom12Ptr;
typedef Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueueComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocatorComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> CommandListComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12Fence> FenceComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12Resource> DeviceResCom12Ptr;
typedef Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3D12StateObject> RTPipelineStateComPtr;
typedef Microsoft::WRL::ComPtr<ID3DBlob> BlobComPtr;

namespace forward
{
	//D3D12_SUBRESOURCE_DATA ConvertSubResource(const Subresource* pData);

	u32 CalcConstantBufferByteSize(u32 byteSize);

	EResult FillInitDataDX12(u32 width, u32 height, u32 depth, u32 mipCount, u32 arraySize, DataFormatType format,
		u32 maxSize, u32 bitSize, const u8* bitData, u32& twidth, u32& theight, u32& tdepth, u32& skipMip, D3D12_SUBRESOURCE_DATA* initData);

	DeviceResCom12Ptr CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		DeviceResCom12Ptr& uploadBuffer);

	// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
	// geometries are stored in one vertex and index buffer.  It provides the offsets
	// and data needed to draw a subset of geometry stores in the vertex and index 
	// buffers so that we can implement the technique described by Figure 6.3.
	struct SubmeshGeometry
	{
		u32 IndexCount = 0;
		u32 StartIndexLocation = 0;
		i32 BaseVertexLocation = 0;

		// Bounding box of the geometry defined by this submesh. 
		// This is used in later chapters of the book.
		//DirectX::BoundingBox Bounds;
	};

	struct MeshGeometry
	{
		// Give it a name so we can look it up by name.
		std::string Name;

		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.  
		Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

		DeviceResCom12Ptr VertexBufferGPU = nullptr;
		DeviceResCom12Ptr IndexBufferGPU = nullptr;

		DeviceResCom12Ptr VertexBufferUploader = nullptr;
		DeviceResCom12Ptr IndexBufferUploader = nullptr;

		// Data about the buffers.
		u32 VertexByteStride = 0;
		u32 VertexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		u32 IndexBufferByteSize = 0;

		// A MeshGeometry may store multiple geometries in one vertex/index buffer.
		// Use this container to define the Submesh geometries so we can draw
		// the Submeshes individually.
		std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = VertexByteStride;
			vbv.SizeInBytes = VertexBufferByteSize;

			return vbv;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
			ibv.Format = IndexFormat;
			ibv.SizeInBytes = IndexBufferByteSize;

			return ibv;
		}

		// We can free this memory after we finish upload to the GPU.
		void DisposeUploaders()
		{
			VertexBufferUploader = nullptr;
			IndexBufferUploader = nullptr;
		}
	};
}