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
#include <mutex>

namespace forward
{
	class SwapChainConfig;
	class SwapChain;
	class DeviceResourceDX12;
	struct PipelineStateObject;


	// This is an unbounded resource descriptor allocator.  It is intended to provide space for CPU-visible resource descriptors
	// as resources are created.  For those that need to be made shader-visible, they will need to be copied to a UserDescriptorHeap
	// or a DynamicDescriptorHeap.
	class DescriptorAllocator
	{
	public:
		DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) 
			: m_Type(Type)
			, m_CurrentHeap(nullptr) 
			, m_DescriptorSize(0)
			, m_RemainingFreeHandles(0)
		{}

		D3D12_CPU_DESCRIPTOR_HANDLE Allocate(u32 Count, ID3D12Device* device);

		static void DestroyAll(void);

	protected:

		static const u32 sm_NumDescriptorsPerHeap = 256;
		static std::mutex sm_AllocationMutex;
		static std::vector<DescriptorHeapComPtr> sm_DescriptorHeapPool;
		static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12Device* device);

		D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
		ID3D12DescriptorHeap* m_CurrentHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
		u32 m_DescriptorSize;
		u32 m_RemainingFreeHandles;
	};


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

		virtual shared_ptr<FrameGraphTexture2D> GetDefaultRT() const override;
		virtual shared_ptr<FrameGraphTexture2D> GetDefaultDS() const override;

		///////////////////////////////////////////////////////////////////////

		D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, u32 Count = 1);
		u32 GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

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

		i32 CreateSwapChain( SwapChainConfig* pConfig );

		void CreateCommandObjects();

		void FlushCommandQueue();
		void OnResize();

		void BeginPresent(ID3D12PipelineState* pso);
		void EndPresent();

		void TransitionResource(DeviceResourceDX12* resource, D3D12_RESOURCE_STATES newState);

		void BuildPSO(PipelineStateObject& pso);

		//--------------------------------------------------------
		DeviceResourceDX12* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
		D3D12_VIEWPORT	mScreenViewport;
		D3D12_RECT		mScissorRect;
		//--------------------------------------------------------

        ID3D12Device* GetDevice();

		ID3D12GraphicsCommandList* CommandList() { return m_CommandList.Get(); }
		ID3D12CommandQueue* CommandQueue() { return m_CommandQueue.Get(); }
		void ResetCommandList();

		void PrepareRenderPass(RenderPass& pass);

    protected:
    	// The main API interfaces used in the renderer.
		DeviceCom12Ptr							m_pDevice = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Debug>		m_pDebugger = nullptr;
		D3D_DRIVER_TYPE							m_driverType = D3D_DRIVER_TYPE_NULL;

		SwapChain*	m_SwapChain = nullptr;

		Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;

		CommandQueueComPtr					m_CommandQueue;
		CommandAllocatorComPtr				m_DirectCmdListAlloc;
		CommandListComPtr					m_CommandList;

		DescriptorAllocator					m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = 
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		};

		FenceComPtr							m_pFence = nullptr;
		u32		m_CurrentFence = 0;

		u32 m_CbvSrvUavDescriptorSize	= 0;

        i32							GetUnusedResourceIndex();

        D3D_FEATURE_LEVEL			m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
		DataFormatType				m_BackBufferFormat = DF_R8G8B8A8_UNORM;

		// Set true to use 4X MSAA.  The default is false.
		bool     m_4xMsaaState = false;    // 4X MSAA enabled
		u32      m_4xMsaaQuality = 0;      // quality level of 4X MSAA

		u32		m_width;
		u32		m_height;
	};

	class RendererContext
	{
	public:
		static RendererDX12* GetCurrentRender();
		static void SetCurrentRender(RendererDX12* render);

	private:
		static RendererDX12* CurrentRender;
	};
};