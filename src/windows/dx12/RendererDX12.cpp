//***************************************************************************************
// RendererDX12.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "RendererDX12.h"

#include "Log.h"

#include "dxCommon/DXGIAdapter.h"
#include "dxCommon/DXGIOutput.h"

#include "dxCommon/SwapChainConfig.h"
#include "dxCommon/SwapChain.h"

#include "d3dx12.h"
#include "utilities/Utils.h"

#include "render/ResourceSystem/Texture.h"
#include "render/FrameGraph/FrameGraph.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DevicePipelineStateObjectDX12.h"
#include "utilities/FileSaver.h"

using Microsoft::WRL::ComPtr;

//--------------------------------------------------------------------------------
using namespace forward;

//
// DescriptorAllocator implementation
//
std::mutex DescriptorAllocator::sm_AllocationMutex;
std::vector<DescriptorHeapComPtr> DescriptorAllocator::sm_DescriptorHeapPool;

void DescriptorAllocator::DestroyAll(void)
{
	sm_DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12Device* device)
{
	std::lock_guard<std::mutex> LockGuard(sm_AllocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = sm_NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;

	DescriptorHeapComPtr pHeap;
	HR(device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap)));
	sm_DescriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(u32 Count, ID3D12Device* device)
{
	if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < Count)
	{
		m_CurrentHeap = RequestNewHeap(m_Type, device);
		m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
		m_RemainingFreeHandles = sm_NumDescriptorsPerHeap;

		if (m_DescriptorSize == 0)
			m_DescriptorSize = device->GetDescriptorHandleIncrementSize(m_Type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
	m_CurrentHandle.ptr += Count * m_DescriptorSize;
	m_RemainingFreeHandles -= Count;
	return ret;
}

//--------------------------------------------------------------------------------
RendererDX12::RendererDX12()
{
}
//--------------------------------------------------------------------------------
RendererDX12::~RendererDX12()
{
	DescriptorAllocator::DestroyAll();
}
//--------------------------------------------------------------------------------
D3D_FEATURE_LEVEL RendererDX12::GetAvailableFeatureLevel(D3D_DRIVER_TYPE /*DriverType*/)
{
	D3D_FEATURE_LEVEL FeatureLevel = m_FeatureLevel;

	// If the device has already been created, simply return the feature level.
	// Otherwise output error message.

	if (m_pDevice) 
	{
		FeatureLevel = m_FeatureLevel;
	}
	else 
	{
		Log::Get().Write(L"Failed to determine the available hardware feature level!");
	}

	return(FeatureLevel);
}
//--------------------------------------------------------------------------------
D3D_FEATURE_LEVEL RendererDX12::GetCurrentFeatureLevel()
{
	return(m_FeatureLevel);
}
//--------------------------------------------------------------------------------
u64 RendererDX12::GetAvailableVideoMemory()
{
	// Acquire the DXGI device, then the adapter.
	// TODO: This method needs to be capable of checking on multiple adapters!

	ComPtr<IDXGIDevice> pDXGIDevice;
	ComPtr<IDXGIAdapter> pDXGIAdapter;

	m_pDevice.CopyTo(pDXGIDevice.GetAddressOf());
	pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf());

	// Use the adapter interface to get its description.  Then grab the available
	// video memory based on if there is dedicated or shared memory for the GPU.

	DXGI_ADAPTER_DESC AdapterDesc;
	pDXGIAdapter->GetDesc(&AdapterDesc);

	u64 availableVideoMem = 0;

	if (AdapterDesc.DedicatedVideoMemory)
		availableVideoMem = AdapterDesc.DedicatedVideoMemory;
	else
		availableVideoMem = AdapterDesc.SharedSystemMemory;

	return(availableVideoMem);
}
//--------------------------------------------------------------------------------
bool RendererDX12::InitializeD3D(D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel)
{
	HRESULT hr = S_OK;

#ifdef _DEBUG
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebugger));
	if (FAILED(hr))
	{
		Log::Get().Write(L"create ID3D12Debug failed!");
		return false;
	}
	m_pDebugger->EnableDebugLayer();
#endif

	// Create a factory to enumerate all of the hardware in the system.
	u32 flags = 0;
#if defined(_DEBUG)
	flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_Factory));
	i32 allowTearing = 0;
	m_Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

	// Enumerate all of the adapters in the current system.  This includes all
	// adapters, even the ones that don't support the ID3D11Device interface.

	ComPtr<IDXGIAdapter4> pCurrentAdapter;
	std::vector<DXGIAdapter> vAdapters;

	while (m_Factory->EnumAdapterByGpuPreference(static_cast<u32>(vAdapters.size()), DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&pCurrentAdapter)) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pCurrentAdapter);

		DXGI_ADAPTER_DESC3 desc;
		pCurrentAdapter->GetDesc3(&desc);

		Log::Get().Write(desc.Description);
	}

	// If we are trying to get a hardware device, then loop through all available
	// adapters until we successfully create it.  This is useful for multi-adapter
	// systems, where the built in GPU may or may not be capable of the most recent
	// feature levels.
	//

	if (DriverType == D3D_DRIVER_TYPE_HARDWARE)
	{
		for (auto pAdapter : vAdapters)
		{
			hr = D3D12CreateDevice(
				pAdapter.m_pAdapter.Get(),
				FeatureLevel,
				IID_PPV_ARGS(&m_pDevice));

			if (hr == S_OK)
				break;
		}
	}
	else
	{
		Log::Get().Write(L"only support Hardware device for DirectX12, Renderer initialization failed!");
		return false;
	}

	if (FAILED(hr))
		return false;

	hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(hr))
	{
		Log::Get().Write(L"Fence creation failed!");
		return false;
	}

	m_CbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = static_cast<DXGI_FORMAT>(m_BackBufferFormat);
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, 
		&msQualityLevels, sizeof(msQualityLevels));

	if (FAILED(hr))
	{
		Log::Get().Write(L"Check 4x MSAA failed!");
	}

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	return true;
}
//--------------------------------------------------------------------------------
void RendererDX12::Shutdown()
{
	SAFE_DELETE(m_textRenderPass);
	SAFE_DELETE(m_textFont);

	if (m_SwapChain && m_SwapChain->GetSwapChain())
	{
		m_SwapChain->GetSwapChain()->SetFullscreenState(false, NULL);
	}
	SAFE_DELETE(m_SwapChain);
}
//--------------------------------------------------------------------------------
ID3D12Device* RendererDX12::GetDevice()
{
	return(m_pDevice.Get());
}
//--------------------------------------------------------------------------------
i32	RendererDX12::GetUnusedResourceIndex()
{
	// Initialize return index to -1.
	i32 index = -1;

	// Search for a NULL index location.
	//for (u32 i = 0; i < m_vResources.size(); i++) {
	//	if (m_vResources[i] == NULL) {
	//		index = i;
	//		break;
	//	}
	//}

	// Return either an empty location, or -1 if none exist.
	return(index);
}
//--------------------------------------------------------------------------------
i32 RendererDX12::CreateSwapChain(SwapChainConfig* pConfig)
{
	// Attempt to create the swap chain.
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	HR(m_Factory->CreateSwapChain(m_CommandQueue.Get(), &pConfig->GetSwapChainDesc(), SwapChain.GetAddressOf()));

	std::vector<DeviceTexture2DDX12*> texVector;
	for (auto i = 0; i < SwapChainBufferCount; ++i)
	{
		DeviceResCom12Ptr pSwapChainBuffer;
		HR(SwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapChainBuffer)));

		std::stringstream ss;
		ss << "DefaultRT" << i;
		auto rtPtr = DeviceTexture2DDX12::BuildDeviceTexture2DDX12(ss.str().c_str(), pSwapChainBuffer.Get(), RU_CPU_GPU_BIDIRECTIONAL);
		texVector.push_back(rtPtr);
	}

	// Create the depth/stencil buffer and view.
	auto dsPtr = forward::make_shared<Texture2D>(std::string("DefaultDS"), DF_D24_UNORM_S8_UINT,
		pConfig->GetWidth(), pConfig->GetHeight(), TextureBindPosition::TBP_DS);
	DeviceTexture2DDX12* dsDevicePtr = new DeviceTexture2DDX12(m_pDevice.Get(), dsPtr.get());
	dsPtr->SetDeviceObject(dsDevicePtr);
	assert(m_SwapChain == nullptr);
	m_SwapChain = new forward::SwapChain(SwapChain, texVector[0]->GetFrameGraphTexture2D(), texVector[1]->GetFrameGraphTexture2D(), dsPtr);

	HR(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// Transition the resource from its initial state to be used as a depth buffer.
	TransitionResource(dsDevicePtr, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	HR(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	return 0;
}
//--------------------------------------------------------------------------------
void RendererDX12::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HR(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

	HR(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));

	HR(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_DirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_CommandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	HR(m_CommandList->Close());
}
//--------------------------------------------------------------------------------
void RendererDX12::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	++m_CurrentFence;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	HR(m_CommandQueue->Signal(m_pFence.Get(), m_CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (m_pFence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		HR(m_pFence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}
//--------------------------------------------------------------------------------
void RendererDX12::OnResize()
{
	FlushCommandQueue();

	HR(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	//for (i32 i = 0; i < SwapChainBufferCount; ++i)
	//{
	//	mSwapChainBuffer[i].Reset();
	//}
	//mDepthStencilBuffer.Reset();

}
//--------------------------------------------------------------------------------
DeviceTexture2DDX12* RendererDX12::CurrentBackBuffer(PipelineStateObject& pso) const
{
	if (pso.m_OMState.m_renderTargetResources[0] && pso.m_OMState.m_renderTargetResources[0]->DeviceObject())
	{
		return device_cast<DeviceTexture2DDX12*>(pso.m_OMState.m_renderTargetResources[0]);
	}
	else if (m_SwapChain)
	{
		auto rtPtr = m_SwapChain->GetCurrentRT();
		auto deviceRes = rtPtr->GetDeviceResource();
		DeviceTexture2DDX12* deviceRes12 = dynamic_cast<DeviceTexture2DDX12*>(deviceRes);
		assert(deviceRes12);
		return deviceRes12;
	}

	return nullptr;
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererDX12::CurrentBackBufferView(PipelineStateObject& pso) const
{
	DeviceTexture2DDX12* tex12 = CurrentBackBuffer(pso);
	assert(tex12);
	return tex12->GetRenderTargetViewHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererDX12::DepthStencilView(PipelineStateObject& pso) const
{
	if (pso.m_OMState.m_depthStencilResource && pso.m_OMState.m_depthStencilResource->DeviceObject())
	{
		return device_cast<DeviceTexture2DDX12*>(pso.m_OMState.m_depthStencilResource)->GetDepthStencilViewHandle();
	}
	else if (m_SwapChain)
	{
		auto dsPtr = m_SwapChain->GetCurrentDS();
		auto deviceRes = dsPtr->GetDeviceResource();
		DeviceTexture2DDX12* tex12 = dynamic_cast<DeviceTexture2DDX12*>(deviceRes);
		assert(tex12);
		return tex12->GetDepthStencilViewHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret;
	ret.ptr = 0;
	return ret;
}
//--------------------------------------------------------------------------------
void RendererDX12::ResetCommandList()
{
	m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr);
}
//--------------------------------------------------------------------------------
void RendererDX12::PrepareRenderPass(RenderPass& pass)
{
	auto& pso = pass.GetPSO();

	if (!pso.m_devicePSO)
	{
		pso.m_devicePSO = forward::make_shared<DevicePipelineStateObjectDX12>(this, pso);
	}

	// create & update device constant buffers
	for (auto i = 0U; i < pso.m_VSState.m_constantBuffers.size(); ++i)
	{
		auto cb = pso.m_VSState.m_constantBuffers[i];
		if (cb)
		{
			if (!cb->DeviceObject())
			{
				auto deviceCB = forward::make_shared<DeviceBufferDX12>(GetDevice(), CommandList(), cb.get());
				cb->SetDeviceObject(deviceCB);
			}
			auto deviceCB = device_cast<DeviceBufferDX12*>(cb);
			deviceCB->SyncCPUToGPU();
		}
	}

	for (auto i = 0U; i < pso.m_GSState.m_constantBuffers.size(); ++i)
	{
		auto cb = pso.m_GSState.m_constantBuffers[i];
		if (cb)
		{
			if (!cb->DeviceObject())
			{
				auto deviceCB = forward::make_shared<DeviceBufferDX12>(GetDevice(), CommandList(), cb.get());
				cb->SetDeviceObject(deviceCB);
			}
			auto deviceCB = device_cast<DeviceBufferDX12*>(cb);
			deviceCB->SyncCPUToGPU();
		}
	}

	for (auto i = 0U; i < pso.m_PSState.m_constantBuffers.size(); ++i)
	{
		auto cb = pso.m_PSState.m_constantBuffers[i];
		if (cb)
		{
			if (!cb->DeviceObject())
			{
				auto deviceCB = forward::make_shared<DeviceBufferDX12>(GetDevice(), CommandList(), cb.get());
				cb->SetDeviceObject(deviceCB);
			}
			auto deviceCB = device_cast<DeviceBufferDX12*>(cb);
			deviceCB->SyncCPUToGPU();
		}
	}

	// create & update device vertex buffer
	for (auto i = 0U; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		auto vb = pso.m_IAState.m_vertexBuffers[i];
		if (vb)
		{
			auto deviceVB = device_cast<DeviceBufferDX12*>(vb);
			deviceVB->SyncCPUToGPU();
		}
	}

	// create & update device index buffer
	auto ib = pso.m_IAState.m_indexBuffer;
	if (ib)
	{
		auto deviceIB = device_cast<DeviceBufferDX12*>(ib);
		deviceIB->SyncCPUToGPU();
	}
}
//--------------------------------------------------------------------------------
void RendererDX12::DrawRenderPass(RenderPass& pass)
{
	auto pso = dynamic_cast<DevicePipelineStateObjectDX12*>(pass.GetPSO().m_devicePSO.get());

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	m_CommandList->RSSetViewports(1, &mScreenViewport);
	D3D12_RECT aRects[FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	memset(aRects, 0, sizeof(aRects));
	if (pass.GetPSO().m_RSState.m_rsState.enableScissor)
	{
		for (auto i = 0U; i < pass.GetPSO().m_RSState.m_activeScissorRectNum; ++i)
		{
			auto rect = pass.GetPSO().m_RSState.m_scissorRects[i];
			aRects[i].left = rect.left;
			aRects[i].bottom = rect.height;
			aRects[i].right = rect.width;
			aRects[i].top = rect.top;
		}
		m_CommandList->RSSetScissorRects(pass.GetPSO().m_RSState.m_rsState.enableScissor, aRects);
	}
	else
	{
		m_CommandList->RSSetScissorRects(1, &mScissorRect);
	}

	// Indicate a state transition on the resource usage.
	TransitionResource(CurrentBackBuffer(pass.GetPSO()), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView(pass.GetPSO());
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView(pass.GetPSO());
	m_CommandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
	// Clear the back buffer and depth buffer.
	f32 clearColours[] = { Colors::LightSteelBlue.x, Colors::LightSteelBlue.y, Colors::LightSteelBlue.z, Colors::LightSteelBlue.w };
	if (pass.GetRenderPassFlags() & RenderPass::OF_CLEAN_RT)
	{
		m_CommandList->ClearRenderTargetView(currentBackBufferView, clearColours, 0, nullptr);
	}
	if (pass.GetRenderPassFlags() & RenderPass::OF_CLEAN_DS)
	{
		m_CommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}


	pso->Bind(CommandList());
	BindGPUVisibleHeaps();
	// Draw
	pass.Execute(*this);
	
	// Indicate a state transition on the resource usage.
	TransitionResource(CurrentBackBuffer(pass.GetPSO()), D3D12_RESOURCE_STATE_PRESENT);
}
//--------------------------------------------------------------------------------
void RendererDX12::DeleteResource(ResourcePtr /*ptr*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::OnResize(u32 /*width*/, u32 /*height*/)
{

}
//--------------------------------------------------------------------------------
bool RendererDX12::Initialize(SwapChainConfig& config, bool bOffScreen)
{
	if (!InitializeD3D(D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_12_0))
	{
		Log::Get().Write(L"Could not create hardware device, trying to create the reference device...");

		if (!InitializeD3D(D3D_DRIVER_TYPE_REFERENCE, D3D_FEATURE_LEVEL_11_0))
		{
			return false;
		}
	}

	CreateCommandObjects();

	m_width = config.GetWidth();
	m_height = config.GetHeight();

	if (!bOffScreen)
	{
		// Create a swap chain for the window that we started out with.  This
		// demonstrates using a configuration object for fast and concise object
		// creation.
		CreateSwapChain(&config);
	}

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(m_width);
	mScreenViewport.Height = static_cast<float>(m_height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, static_cast<i32>(m_width), static_cast<i32>(m_height) };

	/// Font stuff
	m_textFont = new FontSegoe_UIW50H12(20);
	m_textRenderPass = new RenderPass(RenderPass::OF_NO_CLEAN,
		[&](RenderPassBuilder& builder, PipelineStateObject& pso) {
		builder << *m_textFont;

		pso.m_RSState.m_rsState.frontCCW = true;

		// setup render states
		auto rsPtr = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultRT");
		pso.m_OMState.m_renderTargetResources[0] = rsPtr;

		auto& target = pso.m_OMState.m_blendState.target[0];
		target.enable = true;
		target.srcColor = BlendState::Mode::BM_SRC_ALPHA;
		target.dstColor = BlendState::Mode::BM_INV_SRC_ALPHA;
	},
		[&](Renderer& render) {
		render.DrawIndexed(m_textFont->GetIndexCount());
	});

	m_currentFrameGraph = nullptr;

	return true;
}
//--------------------------------------------------------------------------------
void RendererDX12::Draw(u32 vertexNum, u32 startVertexLocation)
{
	CommandList()->DrawInstanced(vertexNum, 1, startVertexLocation, 0);
}
//--------------------------------------------------------------------------------
void RendererDX12::DrawIndexed(u32 indexCount)
{
	CommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}
//--------------------------------------------------------------------------------
void RendererDX12::ResolveResource(Texture2D* dst, Texture2D* src)
{
	if (!dst->DeviceObject())
	{
		auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(GetDevice(), dst);
		dst->SetDeviceObject(deviceTex);
	}

	DeviceResourceDX12* dstDX12 = device_cast<DeviceResourceDX12*>(dst);
	auto backStateDst = dstDX12->GetResourceState();
	TransitionResource(dstDX12, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	DeviceResourceDX12* srcDX12 = device_cast<DeviceResourceDX12*>(src);
	auto backStateSrc = srcDX12->GetResourceState();
	TransitionResource(srcDX12, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	
	CommandList()->ResolveSubresource(dstDX12->GetDeviceResource().Get(), 0, srcDX12->GetDeviceResource().Get(), 0, 
		static_cast<DXGI_FORMAT>(dst->GetFormat()));

	TransitionResource(dstDX12, backStateDst);
	TransitionResource(srcDX12, backStateSrc);
}
//--------------------------------------------------------------------------------
void RendererDX12::SaveRenderTarget(const std::wstring& filename, PipelineStateObject& pso)
{
	DeviceTexture2DDX12* deviceRT = CurrentBackBuffer(pso);
	auto rtPtr = deviceRT->GetFrameGraphTexture2D();
	deviceRT->SyncGPUToCPU();

	u8* tempBuffer = new u8[rtPtr->GetNumBytes()];
	memcpy(tempBuffer, rtPtr->GetData(), rtPtr->GetNumBytes());
	if (rtPtr->GetElementSize() >= 3)
	{
		// transform from RGBA to BGRA
		for (auto i = 0U; i < rtPtr->GetNumBytes(); i += rtPtr->GetElementSize())
		{
			std::swap(tempBuffer[i], tempBuffer[i + 2]);
		}
	}

	FileSaver outfile;
	outfile.SaveAsBMP(filename, tempBuffer, rtPtr->GetWidth(), rtPtr->GetHeight());
	SAFE_DELETE_ARRAY(tempBuffer);
}
//--------------------------------------------------------------------------------
void RendererDX12::DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color)
{
	m_textFont->Typeset(m_width, m_height, x, y, color, msg);
	if (m_currentFrameGraph)
	{
		m_currentFrameGraph->DrawRenderPass(m_textRenderPass);
	}
}
//--------------------------------------------------------------------------------
void RendererDX12::BeginDrawFrameGraph(FrameGraph* fg)
{
	Renderer::BeginDrawFrameGraph(fg);
}
//--------------------------------------------------------------------------------
void RendererDX12::EndDrawFrameGraph()
{
	m_currentFrameGraph->LinkInfo();
	//CompileCurrentFrameGraph();

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	HR(m_DirectCmdListAlloc->Reset());
	std::for_each(std::begin(m_DynamicDescriptorHeaps), std::end(m_DynamicDescriptorHeaps), 
		[](DynamicDescriptorHeapDX12& heap) {
		heap.Reset();
	});

	auto renderPassDB = m_currentFrameGraph->GetRenderPassDB();
	for (auto renderPass : renderPassDB)
	{
		PrepareRenderPass(*renderPass.m_renderPass);
	}

	bool bResetCommandList = false;
	for (auto renderPass : renderPassDB)
	{
		if (!bResetCommandList)
		{
			auto pso = dynamic_cast<DevicePipelineStateObjectDX12*>(renderPass.m_renderPass->GetPSO().m_devicePSO.get());
			auto devicePSO = pso->GetDevicePSO();

			// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
			// Reusing the command list reuses memory.
			HR(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), devicePSO));
			bResetCommandList = true;
		}
		PrepareGPUVisibleHeaps(*renderPass.m_renderPass);
		DrawRenderPass(*renderPass.m_renderPass);
	}

	// Done recording commands.
	HR(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	if (m_SwapChain)
	{
		// swap the back and front buffers
		m_SwapChain->Present();
	}

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
	///-EndPresent
	m_currentFrameGraph = nullptr;
}
//--------------------------------------------------------------------------------
void RendererDX12::PrepareGPUVisibleHeaps(RenderPass& pass)
{
	auto& pso = pass.GetPSO();

	for (auto& heap : m_DynamicDescriptorHeaps)
	{
		heap.PrepareDescriptorHandleCache(pso);
	}

	// stage CBVs
	u32 index = 0;
	auto& heap = m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	for (auto i = 0U; i < pso.m_VSState.m_constantBuffers.size(); ++i)
	{
		auto cb = pso.m_VSState.m_constantBuffers[i];
		if (cb)
		{
			auto deviceCB = device_cast<DeviceBufferDX12*>(cb);
			assert(deviceCB);
			heap.StageDescriptors(index++, 0, 1, deviceCB->GetCBViewCPUHandle());
		}
	}

	for (auto i = 0U; i < pso.m_GSState.m_constantBuffers.size(); ++i)
	{
		auto cb = pso.m_GSState.m_constantBuffers[i];
		if (cb)
		{
			auto deviceCB = device_cast<DeviceBufferDX12*>(cb);
			assert(deviceCB);
			heap.StageDescriptors(index++, 0, 1, deviceCB->GetCBViewCPUHandle());
		}
	}

	for (auto i = 0U; i < pso.m_PSState.m_constantBuffers.size(); ++i)
	{
		auto cb = pso.m_PSState.m_constantBuffers[i];
		if (cb)
		{
			auto deviceCB = device_cast<DeviceBufferDX12*>(cb);
			assert(deviceCB);
			heap.StageDescriptors(index++, 0, 1, deviceCB->GetCBViewCPUHandle());
		}
	}

	for (auto i = 0U; i < pso.m_PSState.m_shaderResources.size(); ++i)
	{
		auto res = pso.m_PSState.m_shaderResources[i];
		if (res)
		{
			auto deviceTex = device_cast<DeviceTexture2DDX12*>(res);
			assert(deviceTex);
			heap.StageDescriptors(index++, 0, 1, deviceTex->GetShaderResourceViewHandle());
		}
	}
}
//--------------------------------------------------------------------------------
void RendererDX12::BindGPUVisibleHeaps()
{
	auto& cbvHeap = m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	cbvHeap.CommitStagedDescriptors(CommandList(), &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}
//--------------------------------------------------------------------------------
shared_ptr<Texture2D> RendererDX12::GetDefaultRT() const
{
	if (m_SwapChain)
	{
		return make_shared<Texture2D>("DefaultRT", m_BackBufferFormat, 0, 0, 0);
	}
	else
	{
		// headless mode
		auto rtPtr = forward::make_shared<Texture2D>(std::string("DefaultRT"), DF_R8G8B8A8_UNORM,
			m_width, m_height, TextureBindPosition::TBP_RT);
		rtPtr->SetUsage(ResourceUsage::RU_CPU_GPU_BIDIRECTIONAL);
		return rtPtr;
	}
}

shared_ptr<Texture2D> RendererDX12::GetDefaultDS() const
{
	auto dsPtr = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultDS");
	if (!dsPtr)
	{
		// headless mode
		dsPtr = forward::make_shared<Texture2D>(std::string("DefaultDS"), DF_D32_FLOAT,
			m_width, m_height, TextureBindPosition::TBP_DS);
	}
	return dsPtr;
}
//--------------------------------------------------------------------------------
void RendererDX12::TransitionResource(DeviceResourceDX12* resource, D3D12_RESOURCE_STATES newState)
{
	if (resource->GetResourceState() == newState)
		return;

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->GetDeviceResource().Get(),
		resource->GetResourceState(), newState);
	m_CommandList->ResourceBarrier(1, &barrier);
	resource->SetResourceState(newState);
}
//--------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE RendererDX12::AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, u32 Count/*= 1*/)
{
	return m_DescriptorAllocators[Type].Allocate(Count, m_pDevice.Get());
}
//--------------------------------------------------------------------------------
void RendererDX12::BuildPSO(PipelineStateObject& /*pso*/)
{
	/// TODO: not implement yet
}
//--------------------------------------------------------------------------------
u32 RendererDX12::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	return m_pDevice->GetDescriptorHandleIncrementSize(type);
}
//--------------------------------------------------------------------------------
RendererDX12* RendererContext::CurrentRender = nullptr;
RendererDX12* RendererContext::GetCurrentRender()
{
	assert(CurrentRender);
	return CurrentRender;
}

void RendererDX12::BeginDraw()
{
	HR(m_DirectCmdListAlloc->Reset());
	ResetCommandList();

	auto deviceRT = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentRT());
	auto deviceDS = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentDS());
	TransitionResource(deviceRT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = deviceRT->GetRenderTargetViewHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = deviceDS->GetDepthStencilViewHandle();
	m_CommandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
	// Clear the back buffer and depth buffer.
	f32 clearColours[] = { Colors::LightSteelBlue.x, Colors::LightSteelBlue.y, Colors::LightSteelBlue.z, Colors::LightSteelBlue.w };
	m_CommandList->ClearRenderTargetView(currentBackBufferView, clearColours, 0, nullptr);
	m_CommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_CommandList->RSSetViewports(1, &mScreenViewport);
	m_CommandList->RSSetScissorRects(1, &mScissorRect);

}

void RendererDX12::EndDraw()
{
	auto deviceRT = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentRT());
	TransitionResource(deviceRT, D3D12_RESOURCE_STATE_PRESENT);
	CommandList()->Close();
	ID3D12CommandList* cmdsLists[] = { CommandList() };
	CommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	m_SwapChain->Present();
	FlushCommandQueue();
}

void RendererContext::SetCurrentRender(RendererDX12* render)
{
	CurrentRender = render;
}