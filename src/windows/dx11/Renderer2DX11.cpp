#include "Renderer2DX11.h"
#include "dxCommon/DXGIAdapter.h"
#include "dxCommon/DXGIOutput.h"
#include "dxCommon/SwapChainConfig.h"
#include "dxCommon/d3dUtil.h"
#include "dx11/InputLayout/DeviceInputLayoutDX11.h"
#include "dx11/ShaderSystem/VertexShaderDX11.h"
#include "dx11/ShaderSystem/PixelShaderDX11.h"
#include "dx11/ShaderSystem/GeometryShaderDX11.h"
#include "dx11/ResourceSystem/Buffers/DeviceVertexBufferDX11.h"
#include "dx11/ResourceSystem/Buffers/DeviceIndexBufferDX11.h"
#include "dx11/ResourceSystem/Buffers/DeviceConstantBufferDX11.h"
#include "dx11/ResourceSystem/Textures/DeviceTexture2DDX11.h"
#include "dx11/DrawingStates/DeviceRasterizerStateDX11.h"
#include "dx11/DrawingStates/DeviceDepthStencilStateDX11.h"
#include "dx11/DrawingStates/DeviceBlendStateDX11.h"
#include "dx11/DrawingStates/DeviceSamplerStateDX11.h"
#include "render/ResourceSystem/FrameGraphResource.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "render/FrameGraph/RenderPass.h"
#include "utilities/Utils.h"
#include "utilities/FileSaver.h"

using namespace forward;
using Microsoft::WRL::ComPtr;

Renderer2DX11::~Renderer2DX11()
{
}

RendererAPI Renderer2DX11::GetRendererAPI() const
{
	return RendererAPI::DirectX11;
}

void Renderer2DX11::Shutdown()
{
	SAFE_DELETE(m_textRenderPass);
	SAFE_DELETE(m_textFont);

	for (auto pSwapChain : m_vSwapChains) 
	{
		if (pSwapChain->GetSwapChain()) 
		{
			pSwapChain->GetSwapChain()->SetFullscreenState(false, NULL);
		}
		SAFE_DELETE(pSwapChain);
	}

	FrameGraphObject::CheckMemoryLeak();
	//m_pDebugger->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
}

bool Renderer2DX11::InitializeD3D(D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel)
{
	// Create a factory to enumerate all of the hardware in the system.
	ComPtr<IDXGIFactory1> pFactory;
	HR(CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(pFactory.GetAddressOf())));


	// Enumerate all of the adapters in the current system.  This includes all
	// adapters, even the ones that don't support the ID3D11Device interface.

	ComPtr<IDXGIAdapter1> pCurrentAdapter;
	std::vector<DXGIAdapter> vAdapters;

	while (pFactory->EnumAdapters1(static_cast<u32>(vAdapters.size()), pCurrentAdapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pCurrentAdapter);

		DXGI_ADAPTER_DESC1 desc;
		pCurrentAdapter->GetDesc1(&desc);

		Log::Get().Write(desc.Description);
	}

	// Specify debug
	u32 CreateDeviceFlags = 0;
#ifdef _DEBUG
	CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL level[] = { FeatureLevel };
	D3D_FEATURE_LEVEL CreatedLevel;

	// If we are trying to get a hardware device, then loop through all available
	// adapters until we successfully create it.  This is useful for multi-adapter
	// systems, where the built in GPU may or may not be capable of the most recent
	// feature levels.
	//
	// If it isn't a hardware device, then we simply use nullptr for the adapter with
	// the appropriate driver type.  See the following page for more details on device 
	// creation: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476082%28v=vs.85%29.aspx
	HRESULT hr = S_OK;
	if (DriverType == D3D_DRIVER_TYPE_HARDWARE)
	{
		for (auto pAdapter : vAdapters)
		{
			hr = D3D11CreateDevice(
				pAdapter.m_pAdapter.Get(),
				D3D_DRIVER_TYPE_UNKNOWN,
				nullptr,
				CreateDeviceFlags,
				level,
				1,
				D3D11_SDK_VERSION,
				m_pDevice.GetAddressOf(),
				&CreatedLevel,
				m_pContext.GetAddressOf());

			if (hr == S_OK)
				break;
		}
	}
	else
	{
		hr = D3D11CreateDevice(
			nullptr,
			DriverType,
			nullptr,
			CreateDeviceFlags,
			level,
			1,
			D3D11_SDK_VERSION,
			m_pDevice.GetAddressOf(),
			&CreatedLevel,
			m_pContext.GetAddressOf());
	}

	if (FAILED(hr))
		return false;

	// Get the debugger interface from the device.
	hr = m_pDevice.CopyTo(m_pDebugger.GetAddressOf());

	if (FAILED(hr))
	{
		Log::Get().Write(L"Unable to acquire the ID3D11Debug interface from the device!");
	}

	// Grab a copy of the feature level for use by the rest of the rendering system.
	m_FeatureLevel = m_pDevice->GetFeatureLevel();

	m_pContext.CopyTo(m_pAnnotation.GetAddressOf());

	// Create a query object to be used to gather statistics on the pipeline.
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
	queryDesc.MiscFlags = 0;

	for (i32 i = 0; i < NumQueries; ++i)
	{
		hr = m_pDevice->CreateQuery(&queryDesc, m_Queries[i].GetAddressOf());

		if (FAILED(hr))
		{
			Log::Get().Write(L"Unable to create a query object!");
			Shutdown();
			return false;
		}
	}

	D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS Options;
	m_pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &Options, sizeof(Options));
	if (Options.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
	{
		Log::Get().Write(L"Device supports compute shaders plus raw and structured buffers via shader 4.x");
	}


	D3D11_FEATURE_DATA_THREADING ThreadingOptions;
	m_pDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &ThreadingOptions, sizeof(ThreadingOptions));

	// TODO: Enumerate all of the formats and quality levels available for the given format.
	//       It may be beneficial to allow this query from the user instead of enumerating
	//       all possible formats...
	u32 NumQuality;
	m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &NumQuality);

	return true;
}

i32 Renderer2DX11::CreateSwapChain(SwapChainConfig* pConfig)
{
	// Attempt to create the DXGI Factory.
	ComPtr<IDXGIDevice> pDXGIDevice;
	HRESULT hr = m_pDevice.CopyTo(pDXGIDevice.GetAddressOf());

	ComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(pDXGIAdapter.GetAddressOf()));

	ComPtr<IDXGIFactory> pFactory;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void **>(pFactory.GetAddressOf()));


	// Attempt to create the swap chain.
	ComPtr<IDXGISwapChain> pSwapChain;
	hr = pFactory->CreateSwapChain(m_pDevice.Get(), &pConfig->GetSwapChainDesc(), pSwapChain.GetAddressOf());
	// Release the factory regardless of pass or fail.
	if (FAILED(hr))
	{
		Log::Get().Write(L"Failed to create swap chain!");
		return -1;
	}

	// Acquire the texture interface from the swap chain.
	Texture2DComPtr pSwapChainBuffer;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast< void** >(pSwapChainBuffer.GetAddressOf()));
	if (FAILED(hr))
	{
		Log::Get().Write(L"Failed to get swap chain texture resource!");
		return -1;
	}

	auto rtPtr = DeviceTexture2DDX11::BuildDeviceTexture2DDX11("DefaultRT", pSwapChainBuffer.Get(), RU_CPU_GPU_BIDIRECTIONAL);
	// Next we create a depth buffer for use in the traditional rendering
	// pipeline.
	auto dsPtr = forward::make_shared<FrameGraphTexture2D>(std::string("DefaultDS"), DF_D32_FLOAT,
		pConfig->GetWidth(), pConfig->GetHeight(), TextureBindPosition::TBP_DS);
	DeviceTexture2DDX11* dsDevicePtr = new DeviceTexture2DDX11(m_pDevice.Get(), dsPtr.get());
	dsPtr->SetDeviceObject(dsDevicePtr);
	m_vSwapChains.push_back(new SwapChain(pSwapChain, rtPtr->FrameGraphObject(), dsPtr));

	m_width = pConfig->GetWidth();
	m_height = pConfig->GetHeight();

	return static_cast<i32>(m_vSwapChains.size() - 1);
}

bool Renderer2DX11::Initialize(SwapChainConfig& config, bool bOffScreen)
{
	if (!InitializeD3D(D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_11_0))
	{
		Log::Get().Write(L"Could not create hardware device, trying to create the reference device...");

		if (!InitializeD3D(D3D_DRIVER_TYPE_REFERENCE, D3D_FEATURE_LEVEL_11_0))
		{
			return false;
		}

	}

	m_width = config.GetWidth();
	m_height = config.GetHeight();

	if (!bOffScreen)
	{
		// Create a swap chain for the window that we started out with.  This
		// demonstrates using a configuration object for fast and concise object
		// creation.
		CreateSwapChain(&config);
	}

	m_textFont = new FontArialW600H36(20);
	m_textRenderPass = new RenderPass;

	return true;
}

void Renderer2DX11::DrawRenderPass(RenderPass& pass)
{
	auto& pso = pass.GetPSO();

	// prepare shaders
	if (!pso.m_VSState.m_shader->DeviceObject())
	{
		auto vs = pso.m_VSState.m_shader;
		auto deviceVS = forward::make_shared<VertexShaderDX11>(m_pDevice.Get(), vs.get());
		vs->SetDeviceObject(deviceVS);
	}

	if (!pso.m_PSState.m_shader->DeviceObject())
	{
		auto ps = pso.m_PSState.m_shader;
		auto devicePS = forward::make_shared<PixelShaderDX11>(m_pDevice.Get(), ps.get());
		ps->SetDeviceObject(devicePS);
	}

	if (pso.m_GSState.m_shader && !pso.m_GSState.m_shader->DeviceObject())
	{
		auto gs = pso.m_GSState.m_shader;
		auto deviceGS = forward::make_shared<GeometryShaderDX11>(m_pDevice.Get(), gs.get());
		gs->SetDeviceObject(deviceGS);
	}

	// setup IA
	if (!pso.m_IAState.m_vertexLayout.DeviceObject())
	{
		auto vb = pso.m_IAState.m_vertexBuffers[0];
		auto vs = pso.m_VSState.m_shader;
		auto vfDevice = forward::make_shared<DeviceInputLayoutDX11>(m_pDevice.Get(), vb.get(), vs.get());
		pso.m_IAState.m_vertexLayout.SetDeviceObject(vfDevice);
	}
	DeviceInputLayoutDX11* layout = device_cast<DeviceInputLayoutDX11*>(&pso.m_IAState.m_vertexLayout);
	m_pContext->IASetInputLayout(layout->GetInputLayout().Get());
	m_pContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(pso.m_IAState.m_topologyType));

	for (auto i = 0U; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		auto vb = pso.m_IAState.m_vertexBuffers[i];
		if (vb)
		{
			if (!vb->DeviceObject())
			{
				auto deviceVB = forward::make_shared<DeviceVertexBufferDX11>(m_pDevice.Get(), vb.get());
				vb->SetDeviceObject(deviceVB);
			}

			auto deviceVB = device_cast<DeviceVertexBufferDX11*>(vb);
			deviceVB->SyncCPUToGPU(m_pContext.Get());
			deviceVB->Bind(m_pContext.Get());
		}
	}

	if (pso.m_IAState.m_indexBuffer)
	{
		auto ib = pso.m_IAState.m_indexBuffer;
		if (!ib->DeviceObject())
		{
			auto deviceIB = forward::make_shared<DeviceIndexBufferDX11>(m_pDevice.Get(), ib.get());
			ib->SetDeviceObject(deviceIB);
		}

		auto deviceIB = device_cast<DeviceIndexBufferDX11*>(ib);
		deviceIB->SyncCPUToGPU(m_pContext.Get());
		deviceIB->Bind(m_pContext.Get());
	}
	else
	{
		m_pContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
	}

	// setup VS
	auto vs = device_cast<VertexShaderDX11*>(pso.m_VSState.m_shader);
	vs->Bind(m_pContext.Get());

	for (auto cb : pso.m_VSState.m_constantBuffers)
	{
		if (cb)
		{
			if (!cb->DeviceObject())
			{
				auto deviceCB = forward::make_shared<DeviceConstantBufferDX11>(m_pDevice.Get(), cb.get());
				cb->SetDeviceObject(deviceCB);
			}

			auto deviceCB = device_cast<DeviceConstantBufferDX11*>(cb);
			deviceCB->SyncCPUToGPU(m_pContext.Get());
			vs->BindCBuffer(m_pContext.Get(), 0, deviceCB->GetDXBufferPtr());
		}
	}

	for (auto i = 0U; i < pso.m_VSState.m_shaderResources.size(); ++i)
	{
		auto res = pso.m_VSState.m_shaderResources[i];
		if (res)
		{
			assert(res->DeviceObject());
			auto deviceRes = device_cast<DeviceTexture2DDX11*>(res);
			vs->BindSRView(m_pContext.Get(), i, deviceRes->GetSRView().Get());
		}
	}

	for (auto i = 0U; i < pso.m_VSState.m_samplers.size(); ++i)
	{
		auto samp = pso.m_VSState.m_samplers[i];
		if (samp)
		{
			if (!samp->DeviceObject())
			{
				auto deviceSampler = forward::make_shared<DeviceSamplerStateDX11>(m_pDevice.Get(), samp.get());
				samp->SetDeviceObject(deviceSampler);
			}

			auto deviceSampler = device_cast<DeviceSamplerStateDX11*>(samp);
			vs->BindSampler(m_pContext.Get(), i, deviceSampler->GetSamplerStateDX11());
		}
	}

	// setup gs
	if (pso.m_GSState.m_shader)
	{
		auto gs = device_cast<GeometryShaderDX11*>(pso.m_GSState.m_shader);
		gs->Bind(m_pContext.Get());

		for (auto cb : pso.m_GSState.m_constantBuffers)
		{
			if (cb)
			{
				if (!cb->DeviceObject())
				{
					auto deviceCB = forward::make_shared<DeviceConstantBufferDX11>(m_pDevice.Get(), cb.get());
					cb->SetDeviceObject(deviceCB);
				}

				auto deviceCB = device_cast<DeviceConstantBufferDX11*>(cb);
				deviceCB->SyncCPUToGPU(m_pContext.Get());
				gs->BindCBuffer(m_pContext.Get(), 0, deviceCB->GetDXBufferPtr());
			}
		}

		for (auto i = 0U; i < pso.m_GSState.m_shaderResources.size(); ++i)
		{
			auto res = pso.m_GSState.m_shaderResources[i];
			if (res)
			{
				assert(res->DeviceObject());
				auto deviceRes = device_cast<DeviceTexture2DDX11*>(res);
				gs->BindSRView(m_pContext.Get(), i, deviceRes->GetSRView().Get());
			}
		}

		for (auto i = 0U; i < pso.m_GSState.m_samplers.size(); ++i)
		{
			auto samp = pso.m_GSState.m_samplers[i];
			if (samp)
			{
				if (!samp->DeviceObject())
				{
					auto deviceSampler = forward::make_shared<DeviceSamplerStateDX11>(m_pDevice.Get(), samp.get());
					samp->SetDeviceObject(deviceSampler);
				}

				auto deviceSampler = device_cast<DeviceSamplerStateDX11*>(samp);
				gs->BindSampler(m_pContext.Get(), i, deviceSampler->GetSamplerStateDX11());
			}
		}
	}
	else
	{
		GeometryShaderDX11::Unbind(m_pContext.Get());
	}


	// setup Rasterizer
	if (!pso.m_RSState.m_rsState.DeviceObject())
	{
		auto rs = new DeviceRasterizerStateDX11(m_pDevice.Get(), &pso.m_RSState.m_rsState);
		pso.m_RSState.m_rsState.SetDeviceObject(rs);
	}
	auto rs = device_cast<DeviceRasterizerStateDX11*>(&pso.m_RSState.m_rsState);
	m_pContext->RSSetState(rs->GetRasterizerStateDX11());
	if (!pso.m_RSState.m_activeViewportsNum)
	{
		ViewPort vp;
		vp.Width = static_cast<f32>(m_width);
		vp.Height = static_cast<f32>(m_height);
		pso.m_RSState.AddViewport(vp);
	}
	D3D11_VIEWPORT aViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	for (auto i = 0U; i < pso.m_RSState.m_activeViewportsNum; ++i)
	{
		const auto& vp = pso.m_RSState.m_viewports[i];
		D3D11_VIEWPORT& dx11VP = aViewports[i];
		dx11VP.Width = vp.Width;
		dx11VP.Height = vp.Height;
		dx11VP.TopLeftX = vp.TopLeftX;
		dx11VP.TopLeftY = vp.TopLeftY;
		dx11VP.MinDepth = vp.MinDepth;
		dx11VP.MaxDepth = vp.MaxDepth;
	}
	m_pContext->RSSetViewports(pso.m_RSState.m_activeViewportsNum, aViewports);

	D3D11_RECT aRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	memset(aRects, 0, sizeof(aRects));
	if (pso.m_RSState.m_activeScissorRectNum)
	{
		for (auto i = 0U; i < pso.m_RSState.m_activeScissorRectNum; ++i)
		{
			const auto& rect = pso.m_RSState.m_scissorRects[i];
			D3D11_RECT& rectDX11 = aRects[i];
			rectDX11.left	= rect.left;
			rectDX11.top	= rect.top;
			rectDX11.right	= rect.width;
			rectDX11.bottom	= rect.height;
		}

		m_pContext->RSSetScissorRects(pso.m_RSState.m_activeScissorRectNum, aRects);
	}
	else
	{
		m_pContext->RSSetScissorRects(0, aRects);
	}

	// setup PS
	auto ps = device_cast<PixelShaderDX11*>(pso.m_PSState.m_shader);
	ps->Bind(m_pContext.Get());

	for (auto cb : pso.m_PSState.m_constantBuffers)
	{
		if (cb)
		{
			if (!cb->DeviceObject())
			{
				auto deviceCB = forward::make_shared<DeviceConstantBufferDX11>(m_pDevice.Get(), cb.get());
				cb->SetDeviceObject(deviceCB);
			}

			auto deviceCB = device_cast<DeviceConstantBufferDX11*>(cb);
			deviceCB->SyncCPUToGPU(m_pContext.Get());
			ps->BindCBuffer(m_pContext.Get(), 0, deviceCB->GetDXBufferPtr());
		}
	}

	for (auto i = 0U; i < pso.m_PSState.m_shaderResources.size(); ++i)
	{
		auto res = pso.m_PSState.m_shaderResources[i];
		if (res)
		{
			assert(res->DeviceObject());
			auto deviceRes = device_cast<DeviceTexture2DDX11*>(res);
			ps->BindSRView(m_pContext.Get(), i, deviceRes->GetSRView().Get());
		}
	}

	for (auto i = 0U; i < pso.m_PSState.m_samplers.size(); ++i)
	{
		auto samp = pso.m_PSState.m_samplers[i];
		if (samp)
		{
			if (!samp->DeviceObject())
			{
				auto deviceSampler = forward::make_shared<DeviceSamplerStateDX11>(m_pDevice.Get(), samp.get());
				samp->SetDeviceObject(deviceSampler);
			}

			auto deviceSampler = device_cast<DeviceSamplerStateDX11*>(samp);
			ps->BindSampler(m_pContext.Get(), i, deviceSampler->GetSamplerStateDX11());
		}
	}

	// setup OutputMerger
	if (!pso.m_OMState.m_blendState.DeviceObject())
	{
		auto blendState = new DeviceBlendStateDX11(m_pDevice.Get(), &pso.m_OMState.m_blendState);
		pso.m_OMState.m_blendState.SetDeviceObject(blendState);
	}
	auto blendState = device_cast<DeviceBlendStateDX11*>(&pso.m_OMState.m_blendState);
	auto blendStateDX11 = blendState->GetBlendStateDX11();
	f32 afBlendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pContext->OMSetBlendState(blendStateDX11, afBlendFactors, 0xFFFFFFFF);

	//if (!pso.m_OMState.m_dsState.DeviceObject())
	//{
	//	auto dsState = new DeviceDepthStencilStateDX11(m_pDevice.Get(), &pso.m_OMState.m_dsState);
	//	pso.m_OMState.m_dsState.SetDeviceObject(dsState);
	//}
	//auto dsState = static_cast<DeviceDepthStencilStateDX11*>(pso.m_OMState.m_dsState.DeviceObject());
	//auto dsStateDX11 = dsState->GetDepthStencilStateDX11();
	//m_pContext->OMSetDepthStencilState(dsStateDX11, )

	ID3D11RenderTargetView*	rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
	ID3D11DepthStencilView* dsv = nullptr;
	for (auto i = 0U; i < pso.m_OMState.m_renderTargetResources.size(); ++i)
	{
		auto rt = pso.m_OMState.m_renderTargetResources[i];
		if (rt)
		{
			if (!rt->DeviceObject())
			{
				auto deviceRT = make_shared<DeviceTexture2DDX11>(m_pDevice.Get(), rt.get());
				rt->SetDeviceObject(deviceRT);
			}
			auto rtDevice = device_cast<DeviceTexture2DDX11*>(rt);
			rtvs[i] = rtDevice->GetRTView().Get();
		}
	}
	if (pso.m_OMState.m_depthStencilResource)
	{
		if (!pso.m_OMState.m_depthStencilResource->DeviceObject())
		{
			auto deviceDS = make_shared<DeviceTexture2DDX11>(m_pDevice.Get(), pso.m_OMState.m_depthStencilResource.get());
			pso.m_OMState.m_depthStencilResource->SetDeviceObject(deviceDS);
		}
		auto dsDevice = device_cast<DeviceTexture2DDX11*>(pso.m_OMState.m_depthStencilResource);
		dsv = dsDevice->GetDSView().Get();
	}
	m_pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, dsv);

	if (pass.GetCleanType() == RenderPass::CleanType::CT_Default)
	{
		auto color = Colors::LightSteelBlue;
		f32 clearColours[] = { color.x, color.y, color.z, color.w }; // RGBA
		m_pContext->ClearRenderTargetView(rtvs[0], clearColours);
		m_pContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0U);
	}

	// Draw
	pass.Execute(*this);
	
	if (pass.GetNextRenderPass())
	{
		DrawRenderPass(*pass.GetNextRenderPass());
	}
	else if (m_vSwapChains.empty())
	{
		m_pContext->Flush();
	}
	else
	{
		m_vSwapChains[0]->GetSwapChain()->Present(0, 0);
	}
}

void Renderer2DX11::Draw(u32 vertexNum, u32 startVertexLocation)
{
	m_pContext->Draw(vertexNum, startVertexLocation);
}

void Renderer2DX11::DrawIndexed(u32 indexCount)
{
	m_pContext->DrawIndexed(indexCount, 0, 0);
}

void Renderer2DX11::Present()
{
	m_vSwapChains[0]->GetSwapChain()->Present(0, 0);
}

void Renderer2DX11::ResolveResource(FrameGraphTexture2D* dst, FrameGraphTexture2D* src)
{
	if (!dst->DeviceObject())
	{
		auto deviceDst = make_shared<DeviceTexture2DDX11>(m_pDevice.Get(), dst);
		dst->SetDeviceObject(deviceDst);
	}

	m_pContext->ResolveSubresource(device_cast<DeviceTexture2DDX11*>(dst)->GetDXTexture2DPtr(), 0, 
		device_cast<DeviceTexture2DDX11*>(src)->GetDXTexture2DPtr(), 0, static_cast<DXGI_FORMAT>(dst->GetFormat()));
}

void Renderer2DX11::SaveRenderTarget(const std::wstring& filename)
{
	auto rtPtr = FrameGraphObject::FindFrameGraphObject<FrameGraphTexture2D>("DefaultRT");
	auto devicert = device_cast<DeviceTexture2DDX11*>(rtPtr);
	devicert->SyncGPUToCPU(m_pContext.Get());

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

void Renderer2DX11::DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color)
{
	m_textFont->Typeset(m_width, m_height, x, y, color, msg);
}

void Renderer2DX11::DeleteResource(ResourcePtr /*ptr*/)
{
}

void Renderer2DX11::OnResize(u32 /*width*/, u32 /*height*/)
{

}