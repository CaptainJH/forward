//***************************************************************************************
// DeviceDX12.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once
//--------------------------------------------------------------------------------
#include "PCH.h"

#include "math/Vector2f.h"
#include "math/Vector3f.h"
#include "math/Vector4f.h"
#include "math/Matrix4f.h"

#include "windows/dx12/dx12Util.h"

#include "RHI/Device.h"
#include "windows/dxCommon/SwapChain.h"
#include "RHI/ResourceSystem/GraphicsObject.h"
#include "DynamicDescriptorHeapDX12.h"
#include "RHI/Text/FontSegoe_UIW50H12.h"
#include <dxgi1_6.h>
#include <mutex>

namespace forward
{
	class SwapChainConfig;
	class SwapChain;
	class DeviceResourceDX12;
	class DeviceTexture2DDX12;
	class CommandQueueDX12;


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


    class DeviceDX12 : public Device
    {
    public:
		DeviceDX12();
		DeviceDX12(u32 w, u32 h, SDL_Window* pWin, HWND hwnd = 0);
		DeviceDX12(SDL_Renderer* r);
        virtual ~DeviceDX12();

        // Access to the renderer.  There should only be a single instance
		// of the renderer at any given time.

		RendererAPI GetRendererAPI() const override { return RendererAPI::DirectX12; }

		void DrawRenderPass(RenderPass& pass) override;

		void DeleteResource(ResourcePtr ptr) override;

		void OnResize(u32 width, u32 height) override;

		void SaveRenderTarget(const std::wstring& filename, RenderPass* rp) override;
		void SaveTexture(const std::wstring& filename, Texture2D* tex) override;

		void DrawScreenText(const std::string& msg, i32 x, i32 y, const float4& color) override;

		void BeginDrawFrameGraph(FrameGraph* fg) override;
		void EndDrawFrameGraph() override;

		shared_ptr<Texture2D> GetDefaultRT() const override;
		shared_ptr<Texture2D> GetDefaultDS() const override;
		shared_ptr<Texture2D> GetCurrentSwapChainRT() override;

		void FlushDefaultQueue() override;
		shared_ptr<CommandQueue> MakeCommandQueue(QueueType t, u32 maxCmdListCount) override;
		CommandQueueDX12* GetDefaultQueue();

		///////////////////////////////////////////////////////////////////////

		D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, u32 Count = 1);
		u32 GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

		ID3D12Device5* GetDevice();
		ID3D12GraphicsCommandList4* DeviceCommandList();
		ID3D12CommandQueue* DeviceCommandQueue();
		void TransitionResource(DeviceResourceDX12* resource, D3D12_RESOURCE_STATES newState);
		void BeginDraw();
		void EndDraw();

		static void ReportLiveObjects();

	protected:
		bool Initialize(SwapChainConfig& config, bool bOffScreen) override;

	private:
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

		i32 CreateSwapChain( SwapChainConfig* pConfig );

		void CreateCommandObjects();

		void Shutdown();
		void OnResize();

		void DrawImGui();

		//--------------------------------------------------------
		DeviceTexture2DDX12* CurrentBackBuffer(RenderPass* rp) const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(RenderPass* rp) const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView(RenderPass* rp) const;
		D3D12_VIEWPORT	mScreenViewport;
		D3D12_RECT		mScissorRect;
		//--------------------------------------------------------

		void PrepareRenderPass(RenderPass& pass);

    private:
    	// The main API interfaces used in the renderer.
		DeviceCom12Ptr							m_pDevice = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Debug>		m_pDebugger = nullptr;
		D3D_DRIVER_TYPE							m_driverType = D3D_DRIVER_TYPE_NULL;

		SwapChain*	m_SwapChain = nullptr;

		Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;

		shared_ptr<forward::CommandQueueDX12> m_queue;
		shared_ptr<forward::CommandListDX12> m_guiCmdList;

		DescriptorAllocator					m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = 
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		};

		u32 m_CbvSrvUavDescriptorSize	= 0;

        i32							GetUnusedResourceIndex();

        D3D_FEATURE_LEVEL			m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
		DataFormatType				m_BackBufferFormat = DF_R8G8B8A8_UNORM;

		// Set true to use 4X MSAA.  The default is false.
		bool     m_4xMsaaState = false;    // 4X MSAA enabled
		u32      m_4xMsaaQuality = 0;      // quality level of 4X MSAA

		u32		m_width;
		u32		m_height;

		Font*	m_textFont;
		RenderPass* m_textRenderPass;
	};

};