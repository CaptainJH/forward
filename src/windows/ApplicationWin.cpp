#include <windowsx.h>
#include <iostream>

#include "ApplicationWin.h"
#include "dxCommon/SwapChainConfig.h"

#ifdef USE_LEGACY_RENDERER
#include "dx11_Hieroglyph/Pipeline/PipelineManagerDX11.h"
#include "dx11_Hieroglyph/ResourceSystem/Texture/Texture2dConfigDX11.h"
#else
//#include "dx11/Renderer2DX11.h"
#include "dx12/DeviceDX12.h"
#endif
#include "ProfilingHelper.h"

//--------------------------------------------------------------------------------
using namespace forward;
using namespace std::chrono;
//--------------------------------------------------------------------------------
namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	ApplicationWin* gApplication = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gApplication->MsgProc(hwnd, msg, wParam, lParam);
}

ApplicationWin::ApplicationWin(HINSTANCE hInstance, i32 width, i32 height)
	: mhAppInst(hInstance),
	mMainWndCaption(L"D3D11 Application"),
	mClientWidth(width),
	mClientHeight(height),
	mEnable4xMsaa(false),
	mhMainWnd(0),
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m4xMsaaQuality(0),
	mAppType(AT_Default)
#ifdef USE_LEGACY_RENDERER
	, m_pRender(nullptr)
#endif
	, m_pDevice(nullptr)
{
	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	gApplication = this;
}

ApplicationWin::ApplicationWin(i32 width, i32 height)
	: mMainWndCaption(L"DirectX Application")
	, mClientWidth(width)
	, mClientHeight(height)
	, mEnable4xMsaa(false)
	, mhMainWnd(0)
	, mAppPaused(false)
	, mMinimized(false)
	, mMaximized(false)
	, mResizing(false)
	, m4xMsaaQuality(0)
	, mAppType(AT_Default)
	, m_pDevice(nullptr)
{
	gApplication = this;
	DeviceType = DeviceType::Device_Forward_DX12;
}

ApplicationWin::ApplicationWin(HWND hwnd, i32 width, i32 height)
	: mMainWndCaption(L"D3D12 Application")
	, mClientWidth(width)
	, mClientHeight(height)
	, mEnable4xMsaa(false)
	, mhMainWnd(hwnd)
	, mAppPaused(false)
	, mMinimized(false)
	, mMaximized(false)
	, mResizing(false)
	, m4xMsaaQuality(0)
	, mAppType(AT_Dll)
	, m_pDevice(nullptr)
{
	gApplication = this;
	DeviceType = DeviceType::Device_Forward_DX12;
}

ApplicationWin::ApplicationWin(void* /*dxDevice*/, forward::DeviceType renderType, const char* forwardPath)
	: mMainWndCaption(L"UnityPlugin")
	, mClientWidth(0)
	, mClientHeight(0)
	, mEnable4xMsaa(false)
	, mhMainWnd(NULL)
	, mAppPaused(false)
	, mMinimized(false)
	, mMaximized(false)
	, mResizing(false)
	, m4xMsaaQuality(0)
	, mAppType(AT_UnityPlugin)
	, m_pDevice(nullptr)
	, DeviceType(renderType)
	, mFileSystem(forwardPath)
{
	gApplication = this;
	//m_pRender2 = new RendererDX12(dxDevice);
}

ApplicationWin::~ApplicationWin()
{
	ShutdownRendererComponents();
	Log::Get().Close();
}

void ApplicationWin::SetAppInst(HINSTANCE hInstance)
{
	mhAppInst = hInstance;
}

HINSTANCE ApplicationWin::AppInst()const
{
	return mhAppInst;
}

HWND ApplicationWin::MainWnd()const
{
	return mhMainWnd;
}

f32 ApplicationWin::AspectRatio()const
{
	return static_cast<f32>(mClientWidth) / mClientHeight;
}

i32 ApplicationWin::Run()
{
	if (IsOffScreenRendering())
	{
		std::cout << "Start off-screen rendering ..." << std::endl;
		mTimer.Tick();
		UpdateScene(0.0f);

		const auto saveFolderW = FileSystem::getSingleton().GetSavedFolder() + mMainWndCaption;
		const auto saveFolder = TextHelper::ToAscii(saveFolderW);
		forward::ProfilingHelper::BeginRenderDocCapture(saveFolder.c_str());

		DrawScene();

		forward::ProfilingHelper::EndRenderDocCapture();

		mTimer.Tick();
		f32 mspf = mTimer.Elapsed();

		std::stringstream outs;
		outs.precision(6);
		outs << "rendering finished, took " << mspf << " (ms)";
		std::cout << outs.str() << std::endl;
		PostDrawScene();
		return 0;
	}
	else if (IsDll())
	{
		mTimer.Tick();
		mFrameStatsText = CalculateFrameStats();
		UpdateScene(mTimer.Elapsed());
		DrawScene();
		return 0;
	}

	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (i32)msg.wParam;
}

bool ApplicationWin::Init()
{
	if (IsOffScreenRendering())
	{
		// attach output to console
		HANDLE consoleHandleOut, consoleHandleError;
		if (AttachConsole(ATTACH_PARENT_PROCESS))
		{
			// Redirect unbuffered STDOUT to the console
			consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (consoleHandleOut != INVALID_HANDLE_VALUE)
			{
				freopen("CONOUT$", "w", stdout);
				setvbuf(stdout, NULL, _IONBF, 0);
			}
			else 
			{
				return false;
			}
			// Redirect unbuffered STDERR to the console
			consoleHandleError = GetStdHandle(STD_ERROR_HANDLE);
			if (consoleHandleError != INVALID_HANDLE_VALUE)
			{
				freopen("CONOUT$", "w", stderr);
				setvbuf(stderr, NULL, _IONBF, 0);
			}
			else
			{
				return false;
			}
		}
	}

	if (!IsOffScreenRendering() && !IsDll())
	{
		if (!InitMainWindow())
			return false;
	}

	if (!ConfigureRendererComponents())
		return false;

	return true;
}

void ApplicationWin::OnResize()
{
	m_pDevice->OnResize(mClientWidth, mClientHeight);
}

LRESULT ApplicationWin::MsgProc(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
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
		if (m_pDevice)
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

	case WM_PAINT:
		UpdateRender();
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool ApplicationWin::InitMainWindow()
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window
	// to achieve 100% scaling while still allowing non-client window content to
	// be rendered in a DPI sensitive fashion.
	// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setthreaddpiawarenesscontext
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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
	::RECT R = { 0, 0, mClientWidth, mClientHeight };
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

std::string ApplicationWin::CalculateFrameStats()
{
	auto fps = mTimer.Framerate();
	f32 mspf = mTimer.Elapsed();

	std::ostringstream outs;
	outs.precision(6);
	outs << "FPS: " << fps << "    "
		<< "Frame Time: " << mspf << " ms";

	return outs.str();
}

bool ApplicationWin::ConfigureRendererComponents()
{
	switch (DeviceType)
	{
//	case Renderer_Hieroglyph:
//#ifdef USE_LEGACY_RENDERER
//		m_pRender = new RendererDX11;
//		m_pRender2 = m_pRender;
//#endif
//		break;
//
//	case Renderer_Forward_DX11:
//#ifndef USE_LEGACY_RENDERER
//		m_pRender2 = new Renderer2DX11;
//#endif
//		break;

	case DeviceType::Device_Forward_DX12:
	{
		m_pDevice = new DeviceDX12;
		break;
	}
	default:
		assert(false);
	}

	SwapChainConfig Config(m_pDevice);
	Config.SetWidth(mClientWidth);
	Config.SetHeight(mClientHeight);
	Config.SetOutputWindow(MainWnd());

	if (!m_pDevice->Initialize(Config, IsOffScreenRendering()))
	{
		ShowWindow(MainWnd(), SW_HIDE);
		MessageBox(MainWnd(), L"Could not create a hardware or software Direct3D 11 device - the program will now abort!",
			mMainWndCaption.c_str(), MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
		RequestTermination();
		return false;
	}

	return true;
}
void ApplicationWin::ShutdownRendererComponents()
{
	if (m_pDevice)
	{
		m_pDevice->Shutdown();
		SAFE_DELETE(m_pDevice);
		DeviceDX12::ReportLiveObjects();
	}
	GraphicsObject::CheckMemoryLeak();
}

void ApplicationWin::RequestTermination()
{
	// This triggers the termination of the application
	PostQuitMessage(0);
}

bool ApplicationWin::IsOffScreenRendering() const
{
	return mAppType == AT_OffScreen;
}

bool ApplicationWin::IsDll() const
{
	return mAppType == AT_Dll;
}

void ApplicationWin::JustEnteringMain()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	//_CrtSetBreakAlloc(570);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void ApplicationWin::ParseCmdLine(const char* cmdLine)
{
	//MessageBoxA(NULL, "Boom", "Boom", MB_OK);
	std::vector<std::string> args;
	std::string cmdLineStr(cmdLine);
	std::string temp = cmdLineStr;
	while (temp.length())
	{
		auto index = temp.find_first_of(' ');
		if (index == std::string::npos)
		{
			// the last arg
			args.push_back(temp);
			break;
		}
		else if (index > 0)
		{
			std::string arg = temp.substr(0, index);
			args.push_back(arg);
		}

		temp = temp.substr(index + 1, temp.length() - index - 1);
	}

	bool isBatchMode = std::find_if(args.begin(), args.end(), [](const std::string& s) -> bool {
		return s == "-headless";
		}) != args.end();

	if (isBatchMode)
	{
		mAppType = AT_OffScreen;
	}
	else
	{
		mAppType = AT_Default;
	}
}

void ApplicationWin::UpdateRender()
{
	if (!m_pDevice) return;

	mTimer.Tick();

	if (!mAppPaused)
	{
		mFrameStatsText = CalculateFrameStats();
		UpdateScene(mTimer.Elapsed());
		DrawScene();
	}
	else
	{
		std::this_thread::sleep_for(100ms);
	}
}

void ApplicationWin::AddExternalResource(const char* name, void* res)
{
	m_pDevice->AddExternalResource(name, res);
}