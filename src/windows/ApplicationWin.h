//--------------------------------------------------------------------------------
// Application
//
// This class represents the base application available to the end user.  The 
// Windows Main function is contained withing the .cpp file, and automatically
// checks for an instance of a CApplication class.  If one is not found then the
// program is exited.
//
// The application currently supports Input, Sound, Rendering, Logging, Timing, 
// and profiling.  These are all available to the user when building an 
// application.
//
//--------------------------------------------------------------------------------
#pragma once
//--------------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <imgui/imgui.h>
#include "PCH.h"
#include "dxCommon/d3dUtil.h"
#include "Timer.h"
#include "FileSystem.h"
#include "Log.h"

#include "Vector2f.h"
#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix4f.h"

#include "Device.h"
#include "Utils.h"
#include "FPCamera.h"
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
struct SDL_Window;
struct SDL_Renderer;
namespace forward
{
	enum ApplicationType
	{
		AT_Default,
		AT_OffScreen,
		AT_Dll, 
		AT_UnityPlugin
	};

	class Application
	{
	public:
		Application(HINSTANCE hInstance, i32 width=800, i32 height=600);
		Application(HWND hwnd, i32 width, i32 height);  // called by PyQT5
		Application(i32 width = 800, i32 height = 600); // offscreen rendering
		Application(void* dxDevice, forward::DeviceType renderType, const char* forwardPath); // unity plugin
		virtual ~Application();

		static void JustEnteringMain();

		void SetAppInst(HINSTANCE hInstance);
		void ParseCmdLine(const char* cmdLine);
		HINSTANCE AppInst()const;
		HWND      MainWnd()const;
		f32     AspectRatio()const;

		i32 Run();

		// Framework methods.  Derived client class overrides these methods to 
		// implement specific application requirements.

		virtual bool Init();
		virtual void OnResize();

		// Request an exit from windows
		void RequestTermination();
		virtual LRESULT MsgProc(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);
		void UpdateRender();
		void AddExternalResource(const char* name, void* res);

	protected:
		virtual void UpdateScene(f32 dt) = 0;
		virtual void DrawScene() = 0;
		virtual void PostDrawScene() {}

		// Convenience overrides for handling mouse input.
		virtual void OnMouseDown(WPARAM /*btnState*/, f32 /*x*/, f32 /*y*/);
		virtual void OnMouseUp(WPARAM /*btnState*/, f32 /*x*/, f32 /*y*/);
		virtual void OnMouseMove(WPARAM /*btnState*/, f32 /*x*/, f32 /*y*/);
		virtual void OnEsc() { RequestTermination(); }
		virtual void OnEnter() {}
		virtual void OnSpace() {}
		virtual void OnChar(i8 /*key*/, bool pressed);
		virtual void OnGUI();

	protected:
		bool InitMainWindow();

		virtual bool ConfigureRendererComponents();
		void ShutdownRendererComponents();

		std::string CalculateFrameStats();
		std::string GetFrameStats() const { return mFrameStatsText; }
		std::string mFrameStatsText;

		bool IsOffScreenRendering() const;
		bool IsDll() const;

		void UpdateCameraMovement(f32 dt);
		void OnSDLRendererPass();

	protected:

		HINSTANCE mhAppInst;
		HWND      mhMainWnd;
		bool      mAppPaused;
		bool      mMinimized;
		bool      mMaximized;
		bool      mResizing;
		u32      m4xMsaaQuality;
		f32		mLastMousePos_x;
		f32		mLastMousePos_y;

		Timer mTimer;
		FileSystem mFileSystem;
		FPCamera mFPCamera;

#ifdef USE_LEGACY_RENDERER
		RendererDX11*	m_pRender;
#endif
		Device*		m_pDevice;

		SDL_Window* m_sdlWnd = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;

		// Derived class should set these in derived constructor to customize starting values.
		std::wstring mMainWndCaption;
		i32 mClientWidth;
		i32 mClientHeight;
		bool mEnable4xMsaa;
		ApplicationType mAppType;

		DeviceType DeviceType = DeviceType::Device_Hieroglyph;

		f32 m_cameraWalkSpeed = 0.0f;
		f32 m_cameraStrafeSpeed = 0.0f;
	};
};