#include "Renderer2DX11.h"
#include "dxCommon/DXGIAdapter.h"
#include "dxCommon/DXGIOutput.h"
#include "dxCommon/SwapChainConfig.h"
#include "dx11/InputLayout/DeviceInputLayoutDX11.h"
#include "dx11/ShaderSystem/VertexShaderDX11.h"
#include "dx11/ShaderSystem/PixelShaderDX11.h"
#include "dx11/ResourceSystem/Buffers/DeviceVertexBufferDX11.h"
#include "dx11/ResourceSystem/Buffers/DeviceIndexBufferDX11.h"
#include "dx11/ResourceSystem/Textures/DeviceTexture2DDX11.h"
#include "render/ResourceSystem/FrameGraphResource.h"
#include "render/ResourceSystem/Textures/FrameGraphTexture.h"
#include "render/FrameGraph/RenderPass.h"

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
	for (auto pSwapChain : m_vSwapChains) 
	{
		if (pSwapChain->GetSwapChain()) 
		{
			pSwapChain->GetSwapChain()->SetFullscreenState(false, NULL);
		}
		delete pSwapChain;
	}
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
		hr = m_pDevice->CreateQuery(&queryDesc, &m_Queries[i]);

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

	auto rt_tex = DeviceTexture2DDX11::BuildDeviceTexture2DDX11("DefaultRT", pSwapChainBuffer.Get());
	ResourcePtr rtResourcePtr(rt_tex);
	// With the resource proxy created, create the swap chain wrapper and store it.
	// The resource proxy can then be used later on by the application to get the
	// RTV or texture ID if needed.

	m_vSwapChains.push_back(new SwapChain(pSwapChain, rtResourcePtr));

	ObjectPtr op(rt_tex->GetFrameGraphResource());
	m_fgObjs.push_back(op);

	return static_cast<i32>(m_vSwapChains.size() - 1);
}

bool Renderer2DX11::Initialize(SwapChainConfig& config)
{
	if (!InitializeD3D(D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_11_0))
	{
		Log::Get().Write(L"Could not create hardware device, trying to create the reference device...");

		if (!InitializeD3D(D3D_DRIVER_TYPE_REFERENCE, D3D_FEATURE_LEVEL_11_0))
		{
			return false;
		}

	}

	// Create a swap chain for the window that we started out with.  This
	// demonstrates using a configuration object for fast and concise object
	// creation.
	/*auto swapChainId =*/ CreateSwapChain(&config);

	// Next we create a depth buffer for use in the traditional rendering
	// pipeline.
	auto dsPtr = std::make_shared<FrameGraphTexture2D>(std::string("DefaultDS"), DF_D32_FLOAT,
		config.GetWidth(), config.GetHeight(), TextureBindPosition::TBP_DS);
	m_fgObjs.push_back(dsPtr);
	DeviceTexture2DDX11* dsDevicePtr = new DeviceTexture2DDX11(m_pDevice.Get(), dsPtr.get());
	dsPtr->SetDeviceObject(dsDevicePtr);


	return true;
}

FrameGraphObject* Renderer2DX11::FindFrameGraphObject(const std::string& name)
{
	for (auto ptr : m_fgObjs)
	{
		if (ptr->Name() == name)
		{
			return ptr.get();
		}
	}

	return nullptr;
}

void Renderer2DX11::DrawRenderPass(RenderPass& pass)
{
	auto& pso = pass.GetPSO();
	if (!pso.m_IAState.m_vertexLayout.DeviceObject())
	{
		auto vb = pso.m_IAState.m_vertexBuffers[0];
		auto vs = static_cast<FrameGraphVertexShader*>(pso.m_VSState.m_shader);
		auto vfDevice = new DeviceInputLayoutDX11(m_pDevice.Get(), vb, vs);
		pso.m_IAState.m_vertexLayout.SetDeviceObject(vfDevice);
	}
	DeviceInputLayoutDX11* layout = static_cast<DeviceInputLayoutDX11*>(pso.m_IAState.m_vertexLayout.DeviceObject());
	m_pContext->IASetInputLayout(layout->GetInputLayout().Get());
	m_pContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(pso.m_IAState.m_topologyType));

	ID3D11Buffer* Buffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { nullptr };
	u32 Strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	u32 Offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	u32 i = 0;
	for (; i < pso.m_IAState.m_vertexBuffers.size(); ++i)
	{
		auto vb = pso.m_IAState.m_vertexBuffers[i];
		if (!vb->DeviceObject())
		{
			auto deviceVB = new DeviceVertexBufferDX11(m_pDevice.Get(), vb);
			vb->SetDeviceObject(deviceVB);
		}

		Buffers[i] = static_cast<DeviceVertexBufferDX11*>(vb->DeviceObject())->GetDXBufferPtr();
		Strides[i] = vb->GetElementSize();
		Offsets[i] = 0;
	}

	
	m_pContext->IASetVertexBuffers(0, i, &Buffers[0], Strides, Offsets);

	if (pso.m_IAState.m_indexBuffer)
	{
		auto ib = pso.m_IAState.m_indexBuffer;
		if (!ib->DeviceObject())
		{
			auto deviceIB = new DeviceIndexBufferDX11(m_pDevice.Get(), ib);
			ib->SetDeviceObject(deviceIB);
		}

		DXGI_FORMAT dataFormat = DXGI_FORMAT_R32_UINT;
		DeviceIndexBufferDX11* pib = static_cast<DeviceIndexBufferDX11*>(ib->DeviceObject());
		m_pContext->IASetIndexBuffer(pib->GetDXBufferPtr(), dataFormat, 0);
	}
	else
	{
		m_pContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
	}
}

void Renderer2DX11::DeleteResource(ResourcePtr /*ptr*/)
{
}

void Renderer2DX11::OnResize(u32 /*width*/, u32 /*height*/)
{

}