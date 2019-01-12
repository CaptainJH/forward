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

#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"

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

	hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));

	// Enumerate all of the adapters in the current system.  This includes all
	// adapters, even the ones that don't support the ID3D11Device interface.

	ComPtr<IDXGIAdapter1> pCurrentAdapter;
	std::vector<DXGIAdapter> vAdapters;

	while (m_Factory->EnumAdapters1(static_cast<u32>(vAdapters.size()), pCurrentAdapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pCurrentAdapter);

		DXGI_ADAPTER_DESC1 desc;
		pCurrentAdapter->GetDesc1(&desc);

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

	m_RtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
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
	const auto Width = pConfig->GetSwapChainDesc().BufferDesc.Width;
	const auto Height = pConfig->GetSwapChainDesc().BufferDesc.Height;

	// Attempt to create the swap chain.
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	HR(m_Factory->CreateSwapChain(m_CommandQueue.Get(), &pConfig->GetSwapChainDesc(), SwapChain.GetAddressOf()));

	//m_currentBackBuffer = 0;

	// Acquire the texture interface from the swap chain.

	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	//for (UINT i = 0; i < SwapChainBufferCount; i++)
	//{
	//	HR(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
	//	m_pDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
	//	rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
	//}

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
	auto dsPtr = forward::make_shared<FrameGraphTexture2D>(std::string("DefaultDS"), DF_D24_UNORM_S8_UINT,
		pConfig->GetWidth(), pConfig->GetHeight(), TextureBindPosition::TBP_DS);
	DeviceTexture2DDX12* dsDevicePtr = new DeviceTexture2DDX12(m_pDevice.Get(), dsPtr.get());
	dsPtr->SetDeviceObject(dsDevicePtr);
	assert(m_SwapChain == nullptr);
	m_SwapChain = new forward::SwapChain(SwapChain, texVector[0]->GetFrameGraphTexture2D(), texVector[1]->GetFrameGraphTexture2D(), dsPtr);

	//D3D12_RESOURCE_DESC depthStencilDesc;
	//depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//depthStencilDesc.Alignment = 0;
	//depthStencilDesc.Width = Width;
	//depthStencilDesc.Height = Height;
	//depthStencilDesc.DepthOrArraySize = 1;
	//depthStencilDesc.MipLevels = 1;
	//depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//depthStencilDesc.SampleDesc.Count = pConfig->GetSwapChainDesc().SampleDesc.Count;
	//depthStencilDesc.SampleDesc.Quality = pConfig->GetSwapChainDesc().SampleDesc.Quality;
	//depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//D3D12_CLEAR_VALUE optClear;
	//optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//optClear.DepthStencil.Depth = 1.0f;
	//optClear.DepthStencil.Stencil = 0;

	//CD3DX12_HEAP_PROPERTIES properties(D3D12_HEAP_TYPE_DEFAULT);
	//HR(m_pDevice->CreateCommittedResource(
	//	&properties,
	//	D3D12_HEAP_FLAG_NONE,
	//	&depthStencilDesc,
	//	D3D12_RESOURCE_STATE_COMMON,
	//	&optClear,
	//	IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
	//));

	//// Create descriptor to mip level 0 of entire resource using the format of the resource.
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	//dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//dsvDesc.Texture2D.MipSlice = 0;
	//m_pDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	HR(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// Transition the resource from its initial state to be used as a depth buffer.
	//D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
	//	D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(dsDevicePtr->GetDeviceResource().Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_CommandList->ResourceBarrier(1, &barrier);


	HR(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(Width);
	mScreenViewport.Height = static_cast<float>(Height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, static_cast<i32>(Width), static_cast<i32>(Height) };

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
//void RendererDX12::CreateRtvAndDsvDescriptorHeaps()
//{
//	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
//	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
//	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvHeapDesc.NodeMask = 0;
//
//	auto hr = m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf()));
//	if (FAILED(hr))
//	{
//		Log::Get().Write(L"Create descriptor heap(RTV) failed!");
//	}
//
//	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
//	dsvHeapDesc.NumDescriptors = 1;
//	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
//	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	dsvHeapDesc.NodeMask = 0;
//
//	hr = m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf()));
//	if (FAILED(hr))
//	{
//		Log::Get().Write(L"Create descriptor heap(DSV) failed!");
//	}
//}
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
ID3D12Resource* RendererDX12::CurrentBackBuffer() const
{
	auto rtPtr = m_SwapChain->GetCurrentRT();
	auto deviceRes = rtPtr->GetResource();
	DeviceResourceDX12* deviceRes12 = dynamic_cast<DeviceResourceDX12*>(deviceRes);
	assert(deviceRes12);
	return deviceRes12->GetDeviceResource().Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererDX12::CurrentBackBufferView() const
{
	//return CD3DX12_CPU_DESCRIPTOR_HANDLE(
	//	m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
	//	m_currentBackBuffer,
	//	m_RtvDescriptorSize);
	auto rtPtr = m_SwapChain->GetCurrentRT();
	auto deviceRes = rtPtr->GetResource();
	DeviceTexture2DDX12* tex12 = dynamic_cast<DeviceTexture2DDX12*>(deviceRes);
	assert(tex12);
	return tex12->GetRenderTargetViewHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererDX12::DepthStencilView() const
{
	//return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsPtr = m_SwapChain->GetCurrentDS();
	auto deviceRes = dsPtr->GetResource();
	DeviceTexture2DDX12* tex12 = dynamic_cast<DeviceTexture2DDX12*>(deviceRes);
	assert(tex12);
	return tex12->GetDepthStencilViewHandle();
}
//--------------------------------------------------------------------------------
void RendererDX12::BeginPresent(ID3D12PipelineState* pso)
{
	//==============================================================
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	HR(m_DirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	HR(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), pso));

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	m_CommandList->RSSetViewports(1, &mScreenViewport);
	m_CommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &barrier);

	// Clear the back buffer and depth buffer.
	f32 clearColours[] = { Colors::LightSteelBlue.x, Colors::LightSteelBlue.y, Colors::LightSteelBlue.z, Colors::LightSteelBlue.w };
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), clearColours, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
	m_CommandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
}
//--------------------------------------------------------------------------------
void RendererDX12::EndPresent()
{
	// Indicate a state transition on the resource usage.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &barrier);

	// Done recording commands.
	HR(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	//HR(m_SwapChain->GetSwapChain()->Present(0, 0));
	m_SwapChain->Present();
	//m_currentBackBuffer = (m_currentBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}
//--------------------------------------------------------------------------------
void RendererDX12::ResetCommandList()
{
	m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr);
}
//--------------------------------------------------------------------------------
void RendererDX12::DrawRenderPass(RenderPass& /*pass*/)
{

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
	//CreateRtvAndDsvDescriptorHeaps();

	m_width = config.GetWidth();
	m_height = config.GetHeight();

	if (!bOffScreen)
	{
		// Create a swap chain for the window that we started out with.  This
		// demonstrates using a configuration object for fast and concise object
		// creation.
		CreateSwapChain(&config);
	}

	/// Font stuff


	ResetCommandList();

	return true;
}
//--------------------------------------------------------------------------------
void RendererDX12::Draw(u32 /*vertexNum*/, u32 /*startVertexLocation*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::DrawIndexed(u32 /*indexCount*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::ResolveResource(FrameGraphTexture2D* /*dst*/, FrameGraphTexture2D* /*src*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::SaveRenderTarget(const std::wstring& /*filename*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::DrawScreenText(const std::string& /*msg*/, i32 /*x*/, i32 /*y*/, const Vector4f& /*color*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::BeginDrawFrameGraph(FrameGraph* /*fg*/)
{

}
//--------------------------------------------------------------------------------
void RendererDX12::EndDrawFrameGraph()
{

}
//--------------------------------------------------------------------------------
shared_ptr<FrameGraphTexture2D> RendererDX12::GetDefaultRT() const
{
	return nullptr;
}

shared_ptr<FrameGraphTexture2D> RendererDX12::GetDefaultDS() const
{
	return nullptr;
}
//--------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE RendererDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, u32 Count/*= 1*/)
{
	return m_DescriptorAllocators[Type].Allocate(Count, m_pDevice.Get());
}
//--------------------------------------------------------------------------------
RendererDX12* RendererContext::CurrentRender = nullptr;
RendererDX12* RendererContext::GetCurrentRender()
{
	assert(CurrentRender);
	return CurrentRender;
}

void RendererContext::SetCurrentRender(RendererDX12* render)
{
	CurrentRender = render;
}