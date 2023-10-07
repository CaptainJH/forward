//***************************************************************************************
// DeviceDX12.cpp by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#include "DeviceDX12.h"

#include "Log.h"

#include "dxCommon/DXGIAdapter.h"
#include "dxCommon/DXGIOutput.h"

#include "dxCommon/SwapChainConfig.h"
#include "dxCommon/SwapChain.h"

#include "d3dx12.h"
#include "utilities/Utils.h"

#include "RHI/ResourceSystem/Texture.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DevicePipelineStateObjectDX12.h"
#include "dx12/CommandQueueDX12.h"
#include "dx12/CommandListDX12.h"
#include "utilities/FileSaver.h"
#include "utilities/FileSystem.h"

#include <dxgidebug.h>
#include <DirectXTex.h>

#include "ProfilingHelper.h"

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
}
//--------------------------------------------------------------------------------
DeviceDX12::~DeviceDX12()
{
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
	m_SwapChain = new forward::SwapChain(SwapChain, texVector, dsPtr);

	// Transition the resource from its initial state to be used as a depth buffer.
	TransitionResource(dsDevicePtr, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_queue->ExecuteCommandList([=]() {});
	m_queue->Flush();

	return 0;
}
//--------------------------------------------------------------------------------
void DeviceDX12::CreateCommandObjects()
{
	m_queue = static_cast<CommandQueueDX12*>(MakeCommandQueue(QueueType::Direct).get());
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
DeviceTexture2DDX12* DeviceDX12::CurrentBackBuffer(PSOUnion& pso) const
{
	if (std::holds_alternative<RasterPipelineStateObject>(pso))
	{
		auto& rpso = std::get<RasterPipelineStateObject>(pso);
		if (rpso.m_OMState.m_renderTargetResources[0] && rpso.m_OMState.m_renderTargetResources[0]->DeviceObject())
			return device_cast<DeviceTexture2DDX12*>(rpso.m_OMState.m_renderTargetResources[0]);
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

D3D12_CPU_DESCRIPTOR_HANDLE DeviceDX12::CurrentBackBufferView(PSOUnion& pso) const
{
	DeviceTexture2DDX12* tex12 = CurrentBackBuffer(pso);
	assert(tex12);
	return tex12->GetRenderTargetViewHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceDX12::DepthStencilView(PSOUnion& pso) const
{
	if (std::holds_alternative<RasterPipelineStateObject>(pso))
	{
		auto& rpso = std::get<RasterPipelineStateObject>(pso);
		if (rpso.m_OMState.m_depthStencilResource && rpso.m_OMState.m_depthStencilResource->DeviceObject())
			return device_cast<DeviceTexture2DDX12*>(rpso.m_OMState.m_depthStencilResource)->GetDepthStencilViewHandle();
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
	if (std::holds_alternative<RasterPipelineStateObject>(pass.GetPSO()))
	{
		auto& pso = pass.GetPSO<RasterPipelineStateObject>();

		if (!pso.m_devicePSO)
		{
			pso.m_devicePSO = forward::make_shared<DevicePipelineStateObjectDX12>(this, pass.GetPSO());
		}

		auto pred = [](auto& ptr) { return static_cast<bool>(ptr); };
		const auto cbCounts = std::ranges::count_if(pso.m_VSState.m_constantBuffers, pred)
			+ std::ranges::count_if(pso.m_PSState.m_constantBuffers, pred);

		if (cbCounts == 0 && pso.m_VSState.m_shader && pso.m_PSState.m_shader)
		{
			auto deviceVS = device_cast<ShaderDX12*>(pso.m_VSState.m_shader);
			for (auto& cb : deviceVS->GetCBuffers())
				pso.m_VSState.m_constantBuffers[cb.GetBindPoint()] = make_shared<ConstantBufferBase>(cb.GetName().c_str(), cb.GetNumBytes());

			auto devicePS = device_cast<ShaderDX12*>(pso.m_PSState.m_shader);
			for (auto& cb : devicePS->GetCBuffers())
				pso.m_PSState.m_constantBuffers[cb.GetBindPoint()] = make_shared<ConstantBufferBase>(cb.GetName().c_str(), cb.GetNumBytes());
		}

		// create & update device constant buffers
		for (auto i = 0U; i < pso.m_VSState.m_constantBuffers.size(); ++i)
		{
			auto cb = pso.m_VSState.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}

		for (auto i = 0U; i < pso.m_GSState.m_constantBuffers.size(); ++i)
		{
			auto cb = pso.m_GSState.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}

		for (auto i = 0U; i < pso.m_PSState.m_constantBuffers.size(); ++i)
		{
			auto cb = pso.m_PSState.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
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
	else if (std::holds_alternative<ComputePipelineStateObject>(pass.GetPSO()))
	{
		auto& pso = pass.GetPSO<ComputePipelineStateObject>();

		if (!pso.m_devicePSO)
		{
			pso.m_devicePSO = forward::make_shared<DevicePipelineStateObjectDX12>(this, pass.GetPSO());
		}

		// create & update device constant buffers
		for (auto i = 0U; i < pso.m_CSState.m_constantBuffers.size(); ++i)
		{
			auto cb = pso.m_CSState.m_constantBuffers[i];
			if (cb)
				m_queue->GetCommandListDX12()->SetDynamicConstantBuffer(cb.get());
		}
	}
}
//--------------------------------------------------------------------------------
void DeviceDX12::DrawRenderPass(RenderPass& pass)
{
	auto GetDevicePSO = [](auto&& pso) {
		return dynamic_cast<DevicePipelineStateObjectDX12*>(pso.m_devicePSO.get());
		};

	if (std::holds_alternative<RasterPipelineStateObject>(pass.GetPSO()))
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

		// Indicate a state transition on the resource usage.
		TransitionResource(CurrentBackBuffer(pass.GetPSO()), D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Specify the buffers we are going to render to.
		D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView(pass.GetPSO());
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView(pass.GetPSO());
		DeviceCommandList()->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
		// Clear the back buffer and depth buffer.
		f32 clearColours[] = { Colors::Black.x, Colors::Black.y, Colors::Black.z, Colors::Black.w };
		if (pass.GetRenderPassFlags() & RenderPass::OF_CLEAN_RT)
		{
			DeviceCommandList()->ClearRenderTargetView(currentBackBufferView, clearColours, 0, nullptr);
		}
		if (pass.GetRenderPassFlags() & RenderPass::OF_CLEAN_DS)
		{
			DeviceCommandList()->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}

		m_queue->GetCommandListDX12()->BindGraphicsPSO(*std::visit(GetDevicePSO, pass.GetPSO()));
		//m_queue->GetCommandListDX12()->BindGraphicsDescriptorTableToRootParam();

		// Draw
		pass.Execute(*this);
	
		// Indicate a state transition on the resource usage.
		TransitionResource(CurrentBackBuffer(pass.GetPSO()), D3D12_RESOURCE_STATE_PRESENT);
	}
	else if (std::holds_alternative<ComputePipelineStateObject>(pass.GetPSO()))
	{
		m_queue->GetCommandListDX12()->BindComputePSO(*std::visit(GetDevicePSO, pass.GetPSO()));
		//m_queue->GetCommandListDX12()->BindComputeDescriptorTableToRootParam();

		// Draw
		pass.Execute(*this);
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
		[&](Device& device) {
			device.DrawIndexed(m_textFont->GetIndexCount());
		}, RenderPass::OF_NO_CLEAN);

	m_currentFrameGraph = nullptr;

	return true;
}
//--------------------------------------------------------------------------------
void DeviceDX12::DrawIndexed(u32 indexCount)
{
	DeviceCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}
//--------------------------------------------------------------------------------
void DeviceDX12::ResolveResource(Texture2D* dst, Texture2D* src)
{
	if (!dst->DeviceObject())
	{
		auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(dst, *this);
		dst->SetDeviceObject(deviceTex);
	}

	DeviceResourceDX12* dstDX12 = device_cast<DeviceResourceDX12*>(dst);
	auto backStateDst = dstDX12->GetResourceState();
	TransitionResource(dstDX12, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	DeviceResourceDX12* srcDX12 = device_cast<DeviceResourceDX12*>(src);
	auto backStateSrc = srcDX12->GetResourceState();
	TransitionResource(srcDX12, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	
	DeviceCommandList()->ResolveSubresource(dstDX12->GetDeviceResource().Get(), 0, srcDX12->GetDeviceResource().Get(), 0, 
		static_cast<DXGI_FORMAT>(dst->GetFormat()));

	TransitionResource(dstDX12, backStateDst);
	TransitionResource(srcDX12, backStateSrc);
}
//--------------------------------------------------------------------------------
void DeviceDX12::SaveRenderTarget(const std::wstring& filename, PSOUnion& pso)
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
void DeviceDX12::DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color)
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
		if (m_SwapChain) m_SwapChain->Present();
		});

	m_currentFrameGraph = nullptr;
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
void DeviceDX12::BuildPSO(PSOUnion& /*pso*/)
{
	/// TODO: not implement yet
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
	TransitionResource(deviceRT, D3D12_RESOURCE_STATE_PRESENT);

	GetDefaultQueue()->ExecuteCommandList([=]() {
		m_SwapChain->Present();
		});
	GetDefaultQueue()->Flush();
}

void DeviceDX12::ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();
}

shared_ptr<CommandQueue> DeviceDX12::MakeCommandQueue(QueueType t)
{
	return shared_ptr<CommandQueueDX12>(new CommandQueueDX12(*this, t));
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

CommandQueue& DeviceDX12::GetQueue()
{
	return *m_queue.get();
}

CommandList& DeviceDX12::GetCmdList()
{
	return *GetQueue().GetCommandList().get();
}