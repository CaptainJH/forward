//***************************************************************************************
// UnityPlugin.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************

#ifdef WINDOWS
#include "dx11/dx11Util.h"
#include "dx12/dx12Util.h"
#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"
#include "IUnityGraphicsD3D12.h"
#endif
#include "UnityApplication.h"

using namespace std;

static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphics* s_Graphics = nullptr;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);
static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType);

static bool initialized = false;
static string s_forwardPath;
static UnityApplication* s_forwardInstance = nullptr;

extern "C"
{
    // Unity plugin load event
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        s_UnityInterfaces = unityInterfaces;
        s_Graphics = unityInterfaces->Get<IUnityGraphics>();

        s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

        // Run OnGraphicsDeviceEvent(initialize) manually on plugin load
        // to not miss the event in case the graphics device is already initialized
        OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
    }

    // Unity plugin unload event
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
    {
        s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DoRenderEventAndData(int /*eventId*/, int* /*data*/)
    {
        if (!initialized)
        {
			MessageBoxA(NULL, "Boom", "DoRenderEventAndData", MB_OK);
            //s_UnityRenderLogic.Initialize(s_UnityInterfaces, eventId, data);
            initialized = true;
        }

        //s_UnityRenderLogic.Render(s_UnityInterfaces, eventId, data);
    }

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DoRenderEvent(int /*eventId*/)
	{
		if (!initialized)
		{
			IUnityGraphicsD3D11* d3d11 = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
			auto dev = d3d11->GetDevice();
			s_forwardInstance = new UnityApplication((void*)dev, forward::RendererType::Renderer_Forward_DX11, s_forwardPath.c_str());
			initialized = s_forwardInstance->Init();
			assert(initialized);
		}

		s_forwardInstance->UpdateRender();
	}

    UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return DoRenderEvent;
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetupResources(const char* name, void* resource)
    {
		s_forwardInstance->AddExternalResource(name, resource);
    }

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetupForwardPath(const char* path)
	{
		s_forwardPath = path;
	}
}

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	UnityGfxRenderer currentDeviceType = s_RendererType;

	switch (eventType)
	{
	    case kUnityGfxDeviceEventInitialize:
	    {
		    //TODO: user initialization code
            initialized = false;
		    break;
	    }
	    case kUnityGfxDeviceEventShutdown:
	    {
		    //TODO: user shutdown code
            //s_UnityRenderLogic.Shutdown(s_UnityInterfaces);
            initialized = false;
		    break;
	    }
	    case kUnityGfxDeviceEventBeforeReset:
	    {
		    //TODO: user Direct3D 9 code
		    break;
	    }
	    case kUnityGfxDeviceEventAfterReset:
	    {
		    //TODO: user Direct3D 9 code
		    break;
	    }
	};

	if (currentDeviceType == kUnityGfxRendererD3D11)
	{
		DoEventGraphicsDeviceD3D11(eventType);
	}
}

static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{

	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
        
	}
}
