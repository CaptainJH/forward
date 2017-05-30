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

namespace forward
{
    class RendererDX12 : public Renderer
    {
    public:
        RendererDX12();
        virtual ~RendererDX12();

        // Access to the renderer.  There should only be a single instance
		// of the renderer at any given time.

		static RendererDX12* Get();

		RendererAPI GetRendererAPI() const override { return DirectX12; }

        // Provide the feature level of the current machine.  This can be
		// called before or after the device has been created.

		D3D_FEATURE_LEVEL GetAvailableFeatureLevel( D3D_DRIVER_TYPE DriverType );
		D3D_FEATURE_LEVEL GetCurrentFeatureLevel();

		// Provide an estimate of the available video memory.

		u64 GetAvailableVideoMemory();

        // Renderer initialization and shutdown methods.  These methods
		// obtain and release all of the hardware specific resources that
		// are used during rendering.

		bool Initialize( D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel );
		void Shutdown();

		// These methods provide rendering frame control.  They are closely
		// related to the API for sequencing rendering batches.

		void Present( HWND hWnd = 0, i32 SwapChain = -1, u32 SyncInterval = 0, u32 PresentFlags = 0 );

		// Allow the application to create swap chains

		//i32 CreateSwapChain( SwapChainConfigDX11* pConfig );


        // Each programmable shader stage can be loaded from file, and stored in a list for
		// later use.  Either an application can directly set these values or a render effect
		// can encapsulate the entire pipeline configuration.

		//i32 LoadShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
		//	const std::wstring& model, bool enablelogging = true );

  //      i32 LoadShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
  //          const std::wstring& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging = true );

        ID3D12Device* GetDevice();

    protected:
    	// The main API interfaces used in the renderer.
		DeviceComPtr							m_pDevice = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Debug>		m_pDebugger = nullptr;
		D3D_DRIVER_TYPE							m_driverType = D3D_DRIVER_TYPE_NULL;

		FenceComPtr								m_pFence = nullptr;

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
	};
};