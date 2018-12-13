//***************************************************************************************
// RendererDX12.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
//--------------------------------------------------------------------------------
#include "PCH.h"

#include "Vector2f.h"
#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix4f.h"

#include "dx12Util.h"

#include "render.h"
#include "dxCommon/SwapChain.h"
#include "render/FrameGraph/FrameGraphObject.h"
#include <dxgi1_4.h>

namespace forward
{
	class SwapChainConfig;
	class SwapChain;


    class RendererDX12 : public Renderer
    {
		static const u32 SwapChainBufferCount = 2;

    public:
        RendererDX12();
        virtual ~RendererDX12();

        // Access to the renderer.  There should only be a single instance
		// of the renderer at any given time.

		RendererAPI GetRendererAPI() const override { return DirectX12; }

		void DrawRenderPass(RenderPass& pass) override;

		void DeleteResource(ResourcePtr ptr) override;

		void OnResize(u32 width, u32 height) override;

		bool Initialize(SwapChainConfig& config, bool bOffScreen) override;
		void Shutdown() override;

		void Draw(u32 vertexNum, u32 startVertexLocation = 0) override;
		void DrawIndexed(u32 indexCount) override;

		void ResolveResource(FrameGraphTexture2D* dst, FrameGraphTexture2D* src) override;

		void SaveRenderTarget(const std::wstring& filename) override;

		void DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color) override;

		void BeginDrawFrameGraph(FrameGraph* fg) override;
		void EndDrawFrameGraph() override;

	//private:
	public:
        // Provide the feature level of the current machine.  This can be
		// called before or after the device has been created.

		D3D_FEATURE_LEVEL GetAvailableFeatureLevel( D3D_DRIVER_TYPE DriverType );
		D3D_FEATURE_LEVEL GetCurrentFeatureLevel();

		// Provide an estimate of the available video memory.

		u64 GetAvailableVideoMemory();

        // Renderer initialization and shutdown methods.  These methods
		// obtain and release all of the hardware specific resources that
		// are used during rendering.

		bool InitializeD3D( D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel );

		// These methods provide rendering frame control.  They are closely
		// related to the API for sequencing rendering batches.

		void Present( HWND hWnd = 0, i32 SwapChain = -1, u32 SyncInterval = 0, u32 PresentFlags = 0 );

		i32 CreateSwapChain( SwapChainConfig* pConfig );

		void CreateCommandObjects();
		void CreateRtvAndDsvDescriptorHeaps();

		void FlushCommandQueue();
		void OnResize();

		void BeginPresent(ID3D12PipelineState* pso);
		void EndPresent();

		//--------------------------------------------------------
		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
		i32 m_currentBackBuffer = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
		D3D12_VIEWPORT	mScreenViewport;
		D3D12_RECT		mScissorRect;
		//--------------------------------------------------------


        // Each programmable shader stage can be loaded from file, and stored in a list for
		// later use.  Either an application can directly set these values or a render effect
		// can encapsulate the entire pipeline configuration.

		//i32 LoadShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
		//	const std::wstring& model, bool enablelogging = true );

  //      i32 LoadShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
  //          const std::wstring& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging = true );

        ID3D12Device* GetDevice();

		ID3D12GraphicsCommandList* CommandList() { return m_CommandList.Get(); }
		ID3D12CommandQueue* CommandQueue() { return m_CommandQueue.Get(); }
		void ResetCommandList();

    protected:
    	// The main API interfaces used in the renderer.
		DeviceCom12Ptr							m_pDevice = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Debug>		m_pDebugger = nullptr;
		D3D_DRIVER_TYPE							m_driverType = D3D_DRIVER_TYPE_NULL;

		// In general, all resources and API objects are housed in expandable arrays
		// wrapper objects.  The position within the array is used to provide fast
		// random access to the renderer clients.

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

		Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;

		CommandQueueComPtr					m_CommandQueue;
		CommandAllocatorComPtr				m_DirectCmdListAlloc;
		CommandListComPtr					m_CommandList;

		DescriptorHeapComPtr				m_RtvHeap;
		DescriptorHeapComPtr				m_DsvHeap;

		FenceComPtr							m_pFence = nullptr;
		u32		m_CurrentFence = 0;

		u32 m_RtvDescriptorSize			= 0;
		u32 m_DsvDescriptorSize			= 0;
		u32 m_CbvSrvUavDescriptorSize	= 0;

        i32							GetUnusedResourceIndex();
        //i32							StoreNewResource( ResourceDX12* pResource );

        D3D_FEATURE_LEVEL			m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
		DataFormatType				m_BackBufferFormat = DF_R8G8B8A8_UNORM;

		// Set true to use 4X MSAA.  The default is false.
		bool     m_4xMsaaState = false;    // 4X MSAA enabled
		u32      m_4xMsaaQuality = 0;      // quality level of 4X MSAA

		u32		m_width;
		u32		m_height;
	};
};