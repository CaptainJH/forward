#include "ApplicationDX11.h"


class InitDirect3DApp : public forward::Application
{
public:
	InitDirect3DApp(HINSTANCE hInstance, int width, int height);
	~InitDirect3DApp();

	bool Init();
	void OnResize();

protected:
	void UpdateScene(float dt);
	void DrawScene();
	virtual void OnEsc();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*prevInstance*/,
	PSTR /*cmdLine*/, int /*showCmd*/)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	InitDirect3DApp theApp(hInstance, 1200, 800);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance, int width, int height)
	: forward::Application(hInstance, width, height)
{
}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Init()
{
	if (!forward::Application::Init())
		return false;

	return true;
}

void InitDirect3DApp::OnResize()
{
	forward::Application::OnResize();
}

void InitDirect3DApp::UpdateScene(float /*dt*/)
{

}

void InitDirect3DApp::DrawScene()
{
	//assert(md3dImmediateContext);
	//assert(mSwapChain);

	//md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	//md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//HR(mSwapChain->Present(0, 0));
}

void InitDirect3DApp::OnEsc()
{
	RequestTermination();
}