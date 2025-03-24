//***************************************************************************************
// DeviceDX12.cpp by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_dx12.h>

#include "DeviceDX12.h"

#include "utilities/Log.h"

#include "windows/dxCommon/DXGIAdapter.h"
#include "windows/dxCommon/DXGIOutput.h"

#include "windows/dxCommon/SwapChainConfig.h"
#include "windows/dxCommon/SwapChain.h"

#include "d3dx12.h"
#include "utilities/Utils.h"

#include "RHI/ResourceSystem/Texture.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "windows/dx12/ResourceSystem/DeviceBufferDX12.h"
#include "windows/dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "windows/dx12/ResourceSystem/Textures/DeviceTextureCubeDX12.h"
#include "windows/dx12/DevicePipelineStateObjectDX12.h"
#include "windows/dx12/CommandQueueDX12.h"
#include "windows/dx12/CommandListDX12.h"
#include "utilities/FileSaver.h"
#include "utilities/FileSystem.h"

#include <dxgidebug.h>
#include <DirectXTex.h>

#include <SDL3/SDL.h>

#include "utilities/ProfilingHelper.h"

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
DeviceDX12::DeviceDX12()
{
	SwapChainConfig config;
	if (!Initialize(config, true))
	{
		MessageBox(NULL, L"Could not create a hardware or software Direct3D device - the program will now abort!", L"Error",
			MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
	}
}
DeviceDX12::DeviceDX12(u32 w, u32 h, SDL_Window* pWin, HWND hwnd)
{
	m_sdlWnd = pWin;
	if (!m_sdlWnd)
		EnableImGUI(false);

	SwapChainConfig Config;
	Config.SetWidth(w);
	Config.SetHeight(h);
	Config.SetOutputWindow(hwnd);

	if (!Initialize(Config, hwnd == 0))
	{
		MessageBox(NULL, L"Could not create a hardware or software Direct3D device - the program will now abort!", L"Error",
			MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
	}
}
DeviceDX12::DeviceDX12(SDL_Renderer* r)
	: Device(r) {

	auto rendererProperty = SDL_GetRendererProperties(m_sdlRenderer);
	Microsoft::WRL::ComPtr<ID3D12Device> devicePtr = (ID3D12Device*)SDL_GetPointerProperty(rendererProperty, SDL_PROP_RENDERER_D3D12_DEVICE_POINTER, nullptr);
	devicePtr.As(&m_pDevice);

	m_CbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check if the device supports ray tracing.
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 features = {};
	auto hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features));
	if (FAILED(hr) || features.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		Log::Get().Write(L"Ray tracing not supported!");
		SDL_assert(false);
	}

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

	auto pSwapChain 
		= (IDXGISwapChain1*)SDL_GetPointerProperty(rendererProperty, SDL_PROP_RENDERER_D3D12_SWAPCHAIN_POINTER, nullptr);
	DXGI_SWAP_CHAIN_DESC s_desc = { 0 };
	pSwapChain->GetDesc(&s_desc);

	auto pGraphicsCommandQueue 
		= (ID3D12CommandQueue*)SDL_GetPointerProperty(rendererProperty, SDL_PROP_RENDERER_D3D12_COMMAND_QUEUE_POINTER, nullptr);
	SDL_assert(pGraphicsCommandQueue);
	m_queue = new CommandQueueDX12(*this, pGraphicsCommandQueue, QueueType::Direct, s_desc.BufferCount);

	m_width = s_desc.BufferDesc.Width;
	m_height = s_desc.BufferDesc.Height;

	{
		Vector<DeviceTexture2DDX12*> deviceTexVector;
		for (auto i = 0U; i < s_desc.BufferCount; ++i)
		{
			DeviceResCom12Ptr pSwapChainBuffer;
			hr = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapChainBuffer));
			if (FAILED(hr)) {
				Log::Get().Write(L"Create Swap Chain failed!");
			}

			std::stringstream ss;
			ss << "DefaultRT" << i;
			auto rtPtr = DeviceTexture2DDX12::BuildDeviceTexture2DDX12(*this, ss.str().c_str(), pSwapChainBuffer.Get(), RU_CPU_GPU_BIDIRECTIONAL);
			deviceTexVector.push_back(rtPtr);
		}

		// Create the depth/stencil buffer and view.
		auto dsPtr = forward::make_shared<Texture2D>(std::string("DefaultDS"), DF_D24_UNORM_S8_UINT,
			m_width, m_height, TextureBindPosition::TBP_DS);
		DeviceTexture2DDX12* dsDevicePtr = new DeviceTexture2DDX12(dsPtr.get(), *this);
		dsPtr->SetDeviceObject(dsDevicePtr);
		assert(m_SwapChain == nullptr);
		Vector<shared_ptr<Texture2D>> texVector;
		for (auto tex : deviceTexVector)
			texVector.push_back(tex->GetTexture2D());
		m_SwapChain = new forward::SwapChain(pSwapChain, texVector, dsPtr, m_sdlRenderer);

		// Transition the resource from its initial state to be used as a depth buffer.
		TransitionResource(dsDevicePtr, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_queue->ExecuteCommandList([=]() {});
		m_queue->Flush();
	}

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(m_width);
	mScreenViewport.Height = static_cast<float>(m_height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, static_cast<i32>(m_width), static_cast<i32>(m_height) };

	/// initialize Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	auto fontPathW = FileSystem::getSingleton().GetDataFolder() + L"NotoSans-Regular.ttf";
	io.Fonts->AddFontFromFileTTF(TextHelper::ToAscii(fontPathW).c_str(), 30);
	//ImGui_ImplWin32_Init(s_desc.OutputWindow);
	ImGui_ImplSDL3_InitForD3D(SDL_GetRenderWindow(r));
	m_guiCmdList = new CommandListDX12(*this, QueueType::Direct);
	auto srvHeap = m_guiCmdList->m_DynamicDescriptorHeaps[0].RequestDescriptorHeap(m_pDevice.Get());
	ImGui_ImplDX12_Init(m_pDevice.Get(), s_desc.BufferCount,
		s_desc.BufferDesc.Format,
		srvHeap.Get(),
		// You'll need to designate a descriptor from your descriptor heap for Dear ImGui to use internally for its font texture's SRV
		srvHeap->GetCPUDescriptorHandleForHeapStart(),
		srvHeap->GetGPUDescriptorHandleForHeapStart());

	/// Font stuff
	m_textFont = new FontSegoe_UIW50H12(64);
	m_textRenderPass = new RenderPass(
		[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
			builder << *m_textFont;
			pso.m_RSState.m_rsState.frontCCW = true;

			// setup render states
			pso.m_OMState.m_renderTargetResources[0] = GetDefaultRT();
			pso.m_OMState.m_depthStencilResource = GetDefaultDS();

			auto& target = pso.m_OMState.m_blendState.target[0];
			target.enable = true;
			target.srcColor = BlendState::Mode::BM_SRC_ALPHA;
			target.dstColor = BlendState::Mode::BM_INV_SRC_ALPHA;
		},
		[&](CommandList& cmdList) {
			cmdList.DrawIndexed(m_textFont->GetIndexCount());
		}, RenderPass::OF_NO_CLEAN);

	m_currentFrameGraph = nullptr;
}
//--------------------------------------------------------------------------------
DeviceDX12::~DeviceDX12()
{
	Shutdown();
	DescriptorAllocator::DestroyAll();
}
//--------------------------------------------------------------------------------
D3D_FEATURE_LEVEL DeviceDX12::GetAvailableFeatureLevel(D3D_DRIVER_TYPE /*DriverType*/)
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
D3D_FEATURE_LEVEL DeviceDX12::GetCurrentFeatureLevel()
{
	return(m_FeatureLevel);
}
//--------------------------------------------------------------------------------
u64 DeviceDX12::GetAvailableVideoMemory()
{
	ComPtr<IDXGIAdapter> pDXGIAdapter;
	m_Factory->EnumAdapterByLuid(m_pDevice->GetAdapterLuid(), IID_PPV_ARGS(&pDXGIAdapter));

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
bool DeviceDX12::InitializeD3D(D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel)
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

	m_CbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check if the device supports ray tracing.
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 features = {};
	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features));
	if (FAILED(hr) || features.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		Log::Get().Write(L"Ray tracing not supported!");
		return false;
	}

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
void DeviceDX12::Shutdown()
{
	SAFE_DELETE(m_textRenderPass);
	SAFE_DELETE(m_textFont);

	if (m_sdlWnd) {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	if (m_SwapChain && m_SwapChain->GetSwapChain())
	{
		m_SwapChain->GetSwapChain()->SetFullscreenState(false, NULL);
	}
	SAFE_DELETE(m_SwapChain);
}
//--------------------------------------------------------------------------------
ID3D12Device5* DeviceDX12::GetDevice()
{
	return m_pDevice.Get();
}
//--------------------------------------------------------------------------------
i32	DeviceDX12::GetUnusedResourceIndex()
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
i32 DeviceDX12::CreateSwapChain(SwapChainConfig* pConfig)
{
	// Attempt to create the swap chain.
	Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;
	auto& desc = pConfig->GetSwapChainDesc();
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
	{
		.Width = desc.BufferDesc.Width,
		.Height = desc.BufferDesc.Height,
		.Format = desc.BufferDesc.Format,
		.SampleDesc = desc.SampleDesc,
		.BufferUsage = desc.BufferUsage,
		.BufferCount = desc.BufferCount,
		.SwapEffect = desc.SwapEffect,
		.Flags = desc.Flags
	};
	HR(m_Factory->CreateSwapChainForHwnd(DeviceCommandQueue(), desc.OutputWindow, &swapChainDesc, nullptr, nullptr,
		SwapChain.GetAddressOf()));
	HR(m_Factory->MakeWindowAssociation(desc.OutputWindow, DXGI_MWA_NO_ALT_ENTER));

	Vector<DeviceTexture2DDX12*> deviceTexVector;
	for (auto i = 0; i < SwapChainConfig::SwapChainBufferCount; ++i)
	{
		DeviceResCom12Ptr pSwapChainBuffer;
		HR(SwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapChainBuffer)));

		std::stringstream ss;
		ss << "DefaultRT" << i;
		auto rtPtr = DeviceTexture2DDX12::BuildDeviceTexture2DDX12(*this, ss.str().c_str(), pSwapChainBuffer.Get(), RU_CPU_GPU_BIDIRECTIONAL);
		deviceTexVector.push_back(rtPtr);
	}

	// Create the depth/stencil buffer and view.
	auto dsPtr = forward::make_shared<Texture2D>(std::string("DefaultDS"), DF_D24_UNORM_S8_UINT,
		pConfig->GetWidth(), pConfig->GetHeight(), TextureBindPosition::TBP_DS);
	DeviceTexture2DDX12* dsDevicePtr = new DeviceTexture2DDX12(dsPtr.get(), *this);
	dsPtr->SetDeviceObject(dsDevicePtr);
	assert(m_SwapChain == nullptr);
	Vector<shared_ptr<Texture2D>> texVector;
	for (auto tex : deviceTexVector)
		texVector.push_back(tex->GetTexture2D());
	m_SwapChain = new forward::SwapChain(SwapChain, texVector, dsPtr, nullptr);

	// Transition the resource from its initial state to be used as a depth buffer.
	TransitionResource(dsDevicePtr, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_queue->ExecuteCommandList([=]() {});
	m_queue->Flush();

	return 0;
}
//--------------------------------------------------------------------------------
void DeviceDX12::CreateCommandObjects()
{
	m_queue = static_cast<CommandQueueDX12*>(MakeCommandQueue(QueueType::Direct, SwapChainConfig::SwapChainBufferCount).get());
}
//--------------------------------------------------------------------------------
void DeviceDX12::OnResize()
{
	//FlushCommandQueue();

	m_queue->GetCommandList()->Reset();

	// Release the previous resources we will be recreating.
	//for (i32 i = 0; i < SwapChainBufferCount; ++i)
	//{
	//	mSwapChainBuffer[i].Reset();
	//}
	//mDepthStencilBuffer.Reset();

}
//--------------------------------------------------------------------------------
DeviceTexture2DDX12* DeviceDX12::CurrentBackBuffer(RasterPipelineStateObject* pso) const
{
	if (pso)
	{
		if (pso->m_OMState.m_renderTargetResources[0] && pso->m_OMState.m_renderTargetResources[0]->DeviceObject())
			return device_cast<DeviceTexture2DDX12*>(pso->m_OMState.m_renderTargetResources[0]);
	}

	if (m_SwapChain)
	{
		auto rtPtr = m_SwapChain->GetCurrentRT();
		auto deviceRes = rtPtr->GetDeviceResource();
		DeviceTexture2DDX12* deviceRes12 = dynamic_cast<DeviceTexture2DDX12*>(deviceRes);
		assert(deviceRes12);
		return deviceRes12;
	}

	return nullptr;
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceDX12::CurrentBackBufferView(RasterPipelineStateObject* pso) const
{
	DeviceTexture2DDX12* tex12 = CurrentBackBuffer(pso);
	assert(tex12);
	return tex12->GetRenderTargetViewHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceDX12::DepthStencilView(RasterPipelineStateObject* pso) const
{
	if (pso)
	{
		if (pso->m_OMState.m_depthStencilResource && pso->m_OMState.m_depthStencilResource->DeviceObject())
			return device_cast<DeviceTexture2DDX12*>(pso->m_OMState.m_depthStencilResource)->GetDepthStencilViewHandle();
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
void DeviceDX12::PrepareRenderPass(RenderPass& pass)
{
	if (pass.IsRasterPSO())
	{
		auto& pso = pass.GetPSO<RasterPipelineStateObject>();

		if (!pso.DeviceObject())
		{
			pso.SetDeviceObject(forward::make_shared<DevicePipelineStateObjectDX12>(this, pso, *pass.m_ia_params.m_vertexBuffers[0]));
		}

		auto pred = [](auto& ptr) { return static_cast<bool>(ptr); };
		const auto cbCounts = std::ranges::count_if(pass.m_vs.m_constantBuffers, pred)
			+ std::ranges::count_if(pass.m_ps.m_constantBuffers, pred);

		if (cbCounts == 0 && pso.m_VSState.m_shader && pso.m_PSState.m_shader)
		{
			auto deviceVS = device_cast<ShaderDX12*>(pso.m_VSState.m_shader);
			for (auto& cb : deviceVS->GetCBuffers())
				pass.m_vs.m_constantBuffers[cb.GetBindPoint()] = make_shared<ConstantBufferBase>(cb.GetName().c_str(), cb.GetNumBytes());

			auto devicePS = device_cast<ShaderDX12*>(pso.m_PSState.m_shader);
			for (auto& cb : devicePS->GetCBuffers())
				pass.m_ps.m_constantBuffers[cb.GetBindPoint()] = make_shared<ConstantBufferBase>(cb.GetName().c_str(), cb.GetNumBytes());
		}

		// create & update device constant buffers
		for (auto i = 0U; i < pass.m_vs.m_constantBuffers.size(); ++i)
		{
			auto cb = pass.m_vs.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}

		for (auto i = 0U; i < pass.m_gs.m_constantBuffers.size(); ++i)
		{
			auto cb = pass.m_gs.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}

		for (auto i = 0U; i < pass.m_ps.m_constantBuffers.size(); ++i)
		{
			auto cb = pass.m_ps.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}

		// setup shader resources
		if (pso.m_PSState.m_shader)
		{
			//const auto& allTextures = devicePS->GetTextures();
			//std::sort(pso.m_PSState.m_shaderResources.begin(), pso.m_PSState.m_shaderResources.begin() + allTextures.size(),
			//	[&](auto& lhs, auto& rhs)->bool {
			//		return std::find_if(allTextures.begin(), allTextures.end(), [&](auto p)->bool {
			//			return p.GetName() == lhs->Name();
			//			}) <
			//			std::find_if(allTextures.begin(), allTextures.end(), [&](auto p)->bool {
			//				return p.GetName() == rhs->Name();
			//				});
			//	});
			for (auto i = 0U; i < pass.m_ps.m_shaderResources.size(); ++i)
			{
				auto res = pass.m_ps.m_shaderResources[i];
				if (res && !res->DeviceObject())
				{
					if (dynamic_cast<Texture2D*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dynamic_cast<Texture2D*>(res.get()), *this);
						res->SetDeviceObject(deviceTex);
					}
					else if (dynamic_cast<TextureCube*>(res.get()))
					{
						auto deviceTex = forward::make_shared<DeviceTextureCubeDX12>(dynamic_cast<TextureCube*>(res.get()), *this);
						res->SetDeviceObject(deviceTex);
					}
				}
			}
		}

		// create & update device vertex buffer
		for (auto i = 0U; i < pass.m_ia_params.m_vertexBuffers.size(); ++i)
		{
			auto vb = pass.m_ia_params.m_vertexBuffers[i];
			if (vb)
			{
				if (!vb->DeviceObject())
				{
					auto deviceVB = forward::make_shared<DeviceBufferDX12>(DeviceCommandList(), vb.get(), *this);
					vb->SetDeviceObject(deviceVB);
				}
				auto deviceVB = device_cast<DeviceBufferDX12*>(vb);
				deviceVB->SyncCPUToGPU();
			}
		}

		// create & update device index buffer
		auto ib = pass.m_ia_params.m_indexBuffer;
		if (ib && !ib->DeviceObject())
		{
			auto deviceIB = forward::make_shared<DeviceBufferDX12>(DeviceCommandList(), ib.get(), *this);
			ib->SetDeviceObject(deviceIB);
		}
		if (ib)
		{
			auto deviceIB = device_cast<DeviceBufferDX12*>(ib);
			deviceIB->SyncCPUToGPU();
		}
	}
	else if (pass.IsComputePSO())
	{
		auto& pso = pass.GetPSO<ComputePipelineStateObject>();

		if (!pso.DeviceObject())
		{
			pso.SetDeviceObject(forward::make_shared<DevicePipelineStateObjectDX12>(this, pso));
		}

		// create & update device constant buffers
		for (auto i = 0U; i < pass.m_cs.m_constantBuffers.size(); ++i)
		{
			auto cb = pass.m_cs.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}

		// setup shader resources
		if (pso.m_CSState.m_shader)
		{
			for (auto i = 0U; i < pass.m_cs.m_shaderResources.size(); ++i)
			{
				auto res = pass.m_cs.m_shaderResources[i];
				if (res)
				{
					if (dynamic_cast<Texture2D*>(res.get()) && !res->DeviceObject())
					{
						auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dynamic_cast<Texture2D*>(res.get()), *this);
						res->SetDeviceObject(deviceTex);
					}
					else if (dynamic_cast<TextureCube*>(res.get()) && !res->DeviceObject())
					{
						auto deviceTex = forward::make_shared<DeviceTextureCubeDX12>(dynamic_cast<TextureCube*>(res.get()), *this);
						res->SetDeviceObject(deviceTex);
					}
					else if (res->GetType() == FGOT_STRUCTURED_BUFFER
						|| res->GetType() == FGOT_VERTEX_BUFFER || res->GetType() == FGOT_INDEX_BUFFER)
						res->SetDeviceObject(make_shared<DeviceBufferDX12>(DeviceCommandList(), res.get(), *this));
				}
			}
			for (auto i = 0U; i < pass.m_cs.m_uavShaderRes.size(); ++i)
			{
				auto res = pass.m_cs.m_uavShaderRes[i];
				if (res)
				{
					if (dynamic_cast<Texture2D*>(res.get()) && !res->DeviceObject())
					{
						auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dynamic_cast<Texture2D*>(res.get()), *this);
						res->SetDeviceObject(deviceTex);
					}
				}
			}
		}
	}
	else if (pass.IsRTPSO())
	{
		auto& pso = pass.GetPSO<RTPipelineStateObject>();

		if (!pso.DeviceObject())
			pso.SetDeviceObject(make_shared<DeviceRTPipelineStateObjectDX12>(this, pso));
		else
		{
			for (auto& pCB : pso.m_rtState.m_constantBuffers)
			{
				if (pCB)
					m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(pCB.get());
			}
		}
	}
}
//--------------------------------------------------------------------------------
void DeviceDX12::DrawRenderPass(RenderPass& pass)
{
	if (pass.IsRasterPSO())
	{
		auto& rpso = pass.GetPSO<RasterPipelineStateObject>();
		// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
		DeviceCommandList()->RSSetViewports(1, &mScreenViewport);
		D3D12_RECT aRects[FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		memset(aRects, 0, sizeof(aRects));
		if (rpso.m_RSState.m_rsState.enableScissor)
		{
			for (auto i = 0U; i < rpso.m_RSState.m_activeScissorRectNum; ++i)
			{
				auto rect = rpso.m_RSState.m_scissorRects[i];
				aRects[i].left = rect.left;
				aRects[i].bottom = rect.height;
				aRects[i].right = rect.width;
				aRects[i].top = rect.top;
			}
			DeviceCommandList()->RSSetScissorRects(rpso.m_RSState.m_rsState.enableScissor, aRects);
		}
		else
		{
			DeviceCommandList()->RSSetScissorRects(1, &mScissorRect);
		}

		// Specify the buffers we are going to render to.
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rt = {};
		for (auto& r : rpso.m_OMState.m_renderTargetResources)
		{
			if (!r) break;
			if (r->DeviceObject())
			{
				auto rtD = device_cast<DeviceTexture2DDX12*>(r);
				rt.emplace_back(rtD->GetRenderTargetViewHandle());
				TransitionResource(rtD, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
			else if (r->Name() == "DefaultRT")
			{
				rt.emplace_back(CurrentBackBufferView(&rpso));
				TransitionResource(CurrentBackBuffer(&rpso), D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
		}
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView(&rpso);
		DeviceCommandList()->OMSetRenderTargets(static_cast<u32>(rt.size()), rt.data(), true, &depthStencilView);
		// Clear the back buffer and depth buffer.
		f32 clearColours[] = { Colors::Black.x, Colors::Black.y, Colors::Black.z, 0.0f };
		if (pass.GetRenderPassFlags() & RenderPass::OF_CLEAN_RT)
		{
			for (auto& r : rt)
				DeviceCommandList()->ClearRenderTargetView(r, clearColours, 0, nullptr);
		}
		if (pass.GetRenderPassFlags() & RenderPass::OF_CLEAN_DS)
		{
			auto rtD = device_cast<DeviceTexture2DDX12*>(rpso.m_OMState.m_depthStencilResource);
			TransitionResource(rtD, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			DeviceCommandList()->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}

		auto& devicePSO = *dynamic_cast<DevicePipelineStateObjectDX12*>(rpso.DeviceObject().get());
		m_queue->GetCommandListDX12()->BindRasterPSO(devicePSO, pass);
		pass.Execute(*m_queue->GetCommandList());
	
		// Indicate a state transition on the resource usage.
		if (rpso.m_OMState.m_renderTargetResources[0]->Name() == "DefaultRT")
			TransitionResource(CurrentBackBuffer(&rpso), D3D12_RESOURCE_STATE_PRESENT);
	}
	else if (pass.IsComputePSO())
	{
		auto& cpso = pass.GetPSO<ComputePipelineStateObject>();
		auto& devicePSO = *dynamic_cast<DevicePipelineStateObjectDX12*>(cpso.DeviceObject().get());
		m_queue->GetCommandListDX12()->BindComputePSO(devicePSO);
		pass.Execute(*m_queue->GetCommandList());
	}
	else if (pass.IsRTPSO())
	{
		auto& pso = pass.GetPSO<RTPipelineStateObject>();
		auto& devicePSO = *dynamic_cast<DeviceRTPipelineStateObjectDX12*>(pso.DeviceObject().get());
		m_queue->GetCommandListDX12()->BindRTPSO(devicePSO);
		pass.Execute(*m_queue->GetCommandList());
	}
}
//--------------------------------------------------------------------------------
void DeviceDX12::DeleteResource(ResourcePtr /*ptr*/)
{

}
//--------------------------------------------------------------------------------
void DeviceDX12::OnResize(u32 /*width*/, u32 /*height*/)
{

}
//--------------------------------------------------------------------------------
bool DeviceDX12::Initialize(SwapChainConfig& config, bool bOffScreen)
{
	if (!InitializeD3D(D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_12_1))
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

		/// initialize Dear ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		auto fontPathW = FileSystem::getSingleton().GetDataFolder() + L"NotoSans-Regular.ttf";
		io.Fonts->AddFontFromFileTTF(TextHelper::ToAscii(fontPathW).c_str(), 30);
		ImGui_ImplSDL3_InitForD3D(m_sdlWnd);
		m_guiCmdList = new CommandListDX12(*this, QueueType::Direct);
		auto srvHeap = m_guiCmdList->m_DynamicDescriptorHeaps[0].RequestDescriptorHeap(m_pDevice.Get());
		ImGui_ImplDX12_Init(m_pDevice.Get(), config.GetSwapChainDesc().BufferCount,
			config.GetSwapChainDesc().BufferDesc.Format,
			srvHeap.Get(),
			// You'll need to designate a descriptor from your descriptor heap for Dear ImGui to use internally for its font texture's SRV
			srvHeap->GetCPUDescriptorHandleForHeapStart(),
			srvHeap->GetGPUDescriptorHandleForHeapStart());
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
	m_textFont = new FontSegoe_UIW50H12(64);
	m_textRenderPass = new RenderPass(
		[&](RenderPassBuilder& builder, RasterPipelineStateObject& pso) {
			builder << *m_textFont;
			pso.m_RSState.m_rsState.frontCCW = true;

			// setup render states
			pso.m_OMState.m_renderTargetResources[0] = GetDefaultRT();
			pso.m_OMState.m_depthStencilResource = GetDefaultDS();

			auto& target = pso.m_OMState.m_blendState.target[0];
			target.enable = true;
			target.srcColor = BlendState::Mode::BM_SRC_ALPHA;
			target.dstColor = BlendState::Mode::BM_INV_SRC_ALPHA;
		},
		[&](CommandList& cmdList) {
			cmdList.DrawIndexed(m_textFont->GetIndexCount());
		}, RenderPass::OF_NO_CLEAN);

	m_currentFrameGraph = nullptr;

	return true;
}
//--------------------------------------------------------------------------------
void DeviceDX12::SaveRenderTarget(const std::wstring& filename, RasterPipelineStateObject* pso)
{
	DeviceTexture2DDX12* deviceRT = CurrentBackBuffer(pso);
	auto rtPtr = deviceRT->GetTexture2D();
	SaveTexture(filename, rtPtr.get());
}
//--------------------------------------------------------------------------------
void DeviceDX12::SaveTexture(const std::wstring& filename, Texture2D* tex)
{
	DeviceTexture2DDX12* deviceRT = device_cast<DeviceTexture2DDX12*>(tex);
	deviceRT->SyncGPUToCPU();

	u8* tempBuffer = new u8[tex->GetNumBytes()];
	memcpy(tempBuffer, tex->GetData(), tex->GetNumBytes());
	if (filename.ends_with(L".bmp"))
	{
		if (tex->GetElementSize() >= 3)
		{
			// transform from RGBA to BGRA
			for (auto i = 0U; i < tex->GetNumBytes(); i += tex->GetElementSize())
			{
				std::swap(tempBuffer[i], tempBuffer[i + 2]);
			}
		}

		FileSaver outfile;
		outfile.SaveAsBMP(filename, tempBuffer, tex->GetWidth(), tex->GetHeight());
	}
	else if (filename.ends_with(L".dds"))
	{
		DirectX::Image image = {
			tex->GetWidth(),
			tex->GetHeight(),
			(DXGI_FORMAT)tex->GetFormat(),
			tex->GetWidth() * 4,
			tex->GetNumBytes(),
			tempBuffer
		};

		std::wstring filepath = FileSystem::getSingleton().GetSavedFolder() + filename;
		DirectX::SaveToDDSFile(image, DirectX::DDS_FLAGS_NONE, filepath.c_str());
	}
	SAFE_DELETE_ARRAY(tempBuffer);
}
//--------------------------------------------------------------------------------
void DeviceDX12::DrawScreenText(const std::string& msg, i32 x, i32 y, const float4& color)
{
	m_textFont->Typeset(m_width, m_height, x, y, color, msg);
	if (m_currentFrameGraph)
	{
		m_currentFrameGraph->DrawRenderPass(m_textRenderPass);
	}
}
//--------------------------------------------------------------------------------
void DeviceDX12::BeginDrawFrameGraph(FrameGraph* fg)
{
	Device::BeginDrawFrameGraph(fg);
}
//--------------------------------------------------------------------------------
void DeviceDX12::EndDrawFrameGraph()
{
	m_currentFrameGraph->LinkInfo();
	//CompileCurrentFrameGraph();

	auto renderPassDB = m_currentFrameGraph->GetRenderPassDB();
	for (auto renderPass : renderPassDB)
	{
		PrepareRenderPass(*renderPass.m_renderPass);
	}

	m_queue->GetCommandListDX12()->BindGPUVisibleHeaps();
	for (auto renderPass : renderPassDB)
	{
		m_queue->GetCommandListDX12()->PrepareGPUVisibleHeaps(*renderPass.m_renderPass);
		DrawRenderPass(*renderPass.m_renderPass);
	}
	m_queue->GetCommandListDX12()->CommitStagedDescriptors();

	m_queue->ExecuteCommandList([=]() {
		if (m_SwapChain && !IsImGUIEnabled()) m_SwapChain->Present();
		});

	m_currentFrameGraph = nullptr;

	if (IsImGUIEnabled())
		DrawImGui();
}

//--------------------------------------------------------------------------------
shared_ptr<Texture2D> DeviceDX12::GetDefaultRT() const
{
	if (m_SwapChain)
	{
		return make_shared<Texture2D>("DefaultRT", m_BackBufferFormat, m_width, m_height, 0);
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

shared_ptr<Texture2D> DeviceDX12::GetCurrentSwapChainRT()
{
	return m_SwapChain ? m_SwapChain->GetCurrentRT() : nullptr;
}

shared_ptr<Texture2D> DeviceDX12::GetDefaultDS() const
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
void DeviceDX12::TransitionResource(DeviceResourceDX12* resource, D3D12_RESOURCE_STATES newState)
{
	if (resource->GetResourceState() == newState)
		return;

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->GetDeviceResource().Get(),
		resource->GetResourceState(), newState);
	DeviceCommandList()->ResourceBarrier(1, &barrier);
	resource->SetResourceState(newState);
}
//--------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE DeviceDX12::AllocateCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, u32 Count/*= 1*/)
{
	return m_DescriptorAllocators[Type].Allocate(Count, m_pDevice.Get());
}
//--------------------------------------------------------------------------------
u32 DeviceDX12::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	return m_pDevice->GetDescriptorHandleIncrementSize(type);
}
//--------------------------------------------------------------------------------
void DeviceDX12::BeginDraw()
{
	auto deviceRT = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentRT());
	auto deviceDS = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentDS());
	TransitionResource(deviceRT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = deviceRT->GetRenderTargetViewHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = deviceDS->GetDepthStencilViewHandle();
	DeviceCommandList()->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
	// Clear the back buffer and depth buffer.
	f32 clearColours[] = { Colors::LightSteelBlue.x, Colors::LightSteelBlue.y, Colors::LightSteelBlue.z, Colors::LightSteelBlue.w };
	DeviceCommandList()->ClearRenderTargetView(currentBackBufferView, clearColours, 0, nullptr);
	DeviceCommandList()->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	DeviceCommandList()->RSSetViewports(1, &mScreenViewport);
	DeviceCommandList()->RSSetScissorRects(1, &mScissorRect);

}

void DeviceDX12::EndDraw()
{
	auto deviceRT = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentRT());
	if (!IsImGUIEnabled())
		TransitionResource(deviceRT, D3D12_RESOURCE_STATE_PRESENT);

	GetDefaultQueue()->ExecuteCommandList([=]() {
		if (!IsImGUIEnabled()) m_SwapChain->Present();
		});

	if (IsImGUIEnabled())
		DrawImGui();
	GetDefaultQueue()->Flush();
}

void DeviceDX12::ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();
}

shared_ptr<CommandQueue> DeviceDX12::MakeCommandQueue(QueueType t, u32 maxCmdListCount)
{
	return shared_ptr<CommandQueueDX12>(new CommandQueueDX12(*this, t, maxCmdListCount));
}

ID3D12GraphicsCommandList4* DeviceDX12::DeviceCommandList()
{
	return m_queue->GetCommandListDX12()->GetDeviceCmdListPtr().Get();
}

ID3D12CommandQueue* DeviceDX12::DeviceCommandQueue()
{
	return m_queue->m_CommandQueue.Get();
}

CommandQueueDX12* DeviceDX12::GetDefaultQueue()
{
	return m_queue.get();
}

void DeviceDX12::FlushDefaultQueue()
{
	return GetDefaultQueue()->Flush();
}

void DeviceDX12::DrawImGui()
{
	static u64 lastGuiSignal = 0;
	m_queue->WaitForGPU(lastGuiSignal);
	auto deviceRT = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentRT());
	m_guiCmdList->Reset();
	m_guiCmdList->BindGPUVisibleHeaps();
	auto guiCmdListDX12 = m_guiCmdList->GetDeviceCmdListPtr();
	auto deviceDS = device_cast<DeviceTexture2DDX12*>(m_SwapChain->GetCurrentDS());
	m_guiCmdList->TransitionBarrier(deviceRT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = deviceRT->GetRenderTargetViewHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = deviceDS->GetDepthStencilViewHandle();
	guiCmdListDX12->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
	guiCmdListDX12->RSSetViewports(1, &mScreenViewport);
	guiCmdListDX12->RSSetScissorRects(1, &mScissorRect);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), guiCmdListDX12.Get());
	m_guiCmdList->TransitionBarrier(deviceRT, D3D12_RESOURCE_STATE_PRESENT);
	m_queue->ExecuteCommandList(*m_guiCmdList);
	m_SwapChain->Present();
	lastGuiSignal = m_queue->Signal();
}