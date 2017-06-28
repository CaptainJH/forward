#include <windowsx.h>

#include "ApplicationDX11.h"
#include "Pipeline/PipelineManagerDX11.h"

#include "dxCommon/SwapChainConfig.h"
#include "ResourceSystem/Texture/Texture2dConfigDX11.h"

//--------------------------------------------------------------------------------
using namespace forward;
using namespace std::chrono;
//--------------------------------------------------------------------------------
namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	ApplicationDX11* gApplicationDX11 = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gApplicationDX11->MsgProc(hwnd, msg, wParam, lParam);
}

ApplicationDX11::ApplicationDX11(HINSTANCE hInstance, i32 width, i32 height)
	: mhAppInst(hInstance),
	mMainWndCaption(L"D3D11 Application"),
	md3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	mClientWidth(width),
	mClientHeight(height),
	mEnable4xMsaa(false),
	mhMainWnd(0),
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m4xMsaaQuality(0),
	m_pRender(0)
{
	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	gApplicationDX11 = this;
}

ApplicationDX11::~ApplicationDX11()
{
	ShutdownRendererComponents();
}

HINSTANCE ApplicationDX11::AppInst()const
{
	return mhAppInst;
}

HWND ApplicationDX11::MainWnd()const
{
	return mhMainWnd;
}

f32 ApplicationDX11::AspectRatio()const
{
	return static_cast<f32>(mClientWidth) / mClientHeight;
}

i32 ApplicationDX11::Run()
{
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			mTimer.Tick();

			if (!mAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(mTimer.Elapsed());
				DrawScene();
			}
			else
			{
				std::this_thread::sleep_for(100ms);
			}
		}
	}

	return (i32)msg.wParam;
}

bool ApplicationDX11::Init()
{
	if (!InitMainWindow())
		return false;

	if (!ConfigureRendererComponents())
		return false;

	return true;
}

void ApplicationDX11::OnResize()
{
	m_pRender->ResizeSwapChain(0, mClientWidth, mClientHeight);
	// TODO: ResizeTexture should accept Resource
	//m_pRender->ResizeTexture(m_DepthTarget, mClientWidth, mClientHeight);

	m_pRender->pImmPipeline->ClearRenderTargets();
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_RenderTarget);
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_DepthTarget);
	m_pRender->pImmPipeline->ApplyRenderTargets();

	m_pRender->ResizeViewport(0, mClientWidth, mClientHeight);
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, 0);
}

LRESULT ApplicationDX11::MsgProc(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			//mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			//mTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (m_pRender && m_pRender->GetDevice())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		//mTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		//mTimer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		RequestTermination();
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			OnSpace();
			break;

		case VK_ESCAPE:
			OnEsc();
			break;

		case VK_RETURN:
			OnEnter();
			break;
		};

	case WM_CHAR:
		OnChar(static_cast<i8>(wParam));
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool ApplicationDX11::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	i32 width = R.right - R.left;
	i32 height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

void ApplicationDX11::CalculateFrameStats()
{
	auto fps = mTimer.Framerate();
	f32 mspf = mTimer.Elapsed();

	std::wostringstream outs;
	outs.precision(6);
	outs << mMainWndCaption << L"    "
		<< L"FPS: " << fps << L"    "
		<< L"Frame Time: " << mspf << L" (ms)";
	SetWindowText(mhMainWnd, outs.str().c_str());
}

bool ApplicationDX11::ConfigureRendererComponents()
{
	m_pRender = new RendererDX11;

	if (!m_pRender->Initialize(md3dDriverType, D3D_FEATURE_LEVEL_11_0))
	{
		Log::Get().Write(L"Could not create hardware device, trying to create the reference device...");

		if (!m_pRender->Initialize(D3D_DRIVER_TYPE_REFERENCE, D3D_FEATURE_LEVEL_11_0))
		{
			ShowWindow(MainWnd(), SW_HIDE);
			MessageBox(MainWnd(), L"Could not create a hardware or software Direct3D 11 device - the program will now abort!", 
				mMainWndCaption.c_str(), MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
			RequestTermination();
			return false;
		}

	}

	// Create a swap chain for the window that we started out with.  This
	// demonstrates using a configuration object for fast and concise object
	// creation.

	SwapChainConfig Config(m_pRender);
	Config.SetWidth(mClientWidth);
	Config.SetHeight(mClientHeight);
	Config.SetOutputWindow(MainWnd());
	auto swapChainId = m_pRender->CreateSwapChain(&Config);

	// We'll keep a copy of the render target index to use in later examples.

	m_RenderTarget = m_pRender->GetSwapChainResource(swapChainId);

	// Next we create a depth buffer for use in the traditional rendering
	// pipeline.

	//Texture2dConfigDX11 DepthConfig;
	//DepthConfig.SetDepthBuffer(mClientWidth, mClientHeight);
	TextureDSConfig DepthConfig;
	DepthConfig.SetWidth(mClientWidth);
	DepthConfig.SetHeight(mClientHeight);
	DepthConfig.SetFormat(DF_D32_FLOAT);
	m_DepthTarget = m_pRender->CreateTexture2D(&DepthConfig, 0);
	m_DepthTarget->SetName("DepthStencilTarget");

	// Bind the swap chain render target and the depth buffer for use in 
	// rendering.  

	m_pRender->pImmPipeline->ClearRenderTargets();
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetResources.SetState(0, m_RenderTarget);
	m_pRender->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetResources.SetState(m_DepthTarget);
	m_pRender->pImmPipeline->ApplyRenderTargets();


	// Create a view port to use on the scene.  This basically selects the 
	// entire floating point area of the render target.
	i32 ViewPort = m_pRender->CreateViewPort(mClientWidth, mClientHeight);
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
	m_pRender->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, ViewPort);

	return true;
}
void ApplicationDX11::ShutdownRendererComponents()
{
	if (m_pRender)
	{
		m_pRender->Shutdown();
		SAFE_DELETE(m_pRender);
	}
}

void ApplicationDX11::RequestTermination()
{
	// This triggers the termination of the application
	PostQuitMessage(0);
}