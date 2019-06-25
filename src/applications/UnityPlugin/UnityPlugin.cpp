//***************************************************************************************
// UnityPlugin.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************

#include "Application.h"
#include "dx11/dx11Util.h"
#include "dx12/dx12Util.h"
#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"
#include "IUnityGraphicsD3D12.h"
//#include "UnityRenderLogic.h"
using namespace std;

static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphics* s_Graphics = nullptr;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);
static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType);

static bool initialized = false;

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
            //s_UnityRenderLogic.Initialize(s_UnityInterfaces, eventId, data);
            initialized = true;
        }

        //s_UnityRenderLogic.Render(s_UnityInterfaces, eventId, data);
    }

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DoRenderEvent(int /*eventId*/)
	{

	}

    UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return DoRenderEvent;
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetupResources(void* /*indexBuffer*/, void* /*vertexBuffer*/, void* /*texture*/)
    {
        //s_UnityRenderLogic.SetupResources(indexBuffer, vertexBuffer, texture);
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
