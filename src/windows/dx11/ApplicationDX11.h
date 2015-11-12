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
#ifndef ApplicationDX11_h
#define ApplicationDX11_h
//--------------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "PCH.h"
#include "d3dUtil.h"
#include "RendererDX11.h"
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
namespace forward
{
	class ApplicationDX11
	{
	public:
		ApplicationDX11(HINSTANCE hInstance, int width=800, int height=600);
		virtual ~ApplicationDX11();

		HINSTANCE AppInst()const;
		HWND      MainWnd()const;
		float     AspectRatio()const;

		int Run();

		// Framework methods.  Derived client class overrides these methods to 
		// implement specific application requirements.

		virtual bool Init();
		virtual void OnResize();
		void OnResize2();

		// Request an exit from windows
		void RequestTermination();
		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	protected:
		virtual void UpdateScene(float dt) = 0;
		virtual void DrawScene() = 0;

		// Convenience overrides for handling mouse input.
		virtual void OnMouseDown(WPARAM /*btnState*/, int /*x*/, int /*y*/) { }
		virtual void OnMouseUp(WPARAM /*btnState*/, int /*x*/, int /*y*/) { }
		virtual void OnMouseMove(WPARAM /*btnState*/, int /*x*/, int /*y*/) { }
		virtual void OnEsc() {}
		virtual void OnEnter() {}
		virtual void OnSpace() {}

	protected:
		bool InitMainWindow();

		virtual bool ConfigureRendererComponents();
		virtual void ShutdownRendererComponents();

		void CalculateFrameStats();

	protected:

		HINSTANCE mhAppInst;
		HWND      mhMainWnd;
		bool      mAppPaused;
		bool      mMinimized;
		bool      mMaximized;
		bool      mResizing;
		UINT      m4xMsaaQuality;

		//GameTimer mTimer;

		RendererDX11*	m_pRender;
		ResourcePtr		m_RenderTarget;
		ResourcePtr		m_DepthTarget;

		//ID3D11Device* md3dDevice;
		ID3D11DeviceContext* md3dImmediateContext;
		IDXGISwapChain* mSwapChain;
		ID3D11Texture2D* mDepthStencilBuffer;
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11DepthStencilView* mDepthStencilView;
		D3D11_VIEWPORT mScreenViewport;

		// Derived class should set these in derived constructor to customize starting values.
		std::wstring mMainWndCaption;
		D3D_DRIVER_TYPE md3dDriverType;
		int mClientWidth;
		int mClientHeight;
		bool mEnable4xMsaa;
	};

	typedef ApplicationDX11 Application;
};
//--------------------------------------------------------------------------------
#endif // ApplicationDX11_h
//--------------------------------------------------------------------------------