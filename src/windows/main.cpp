#include "ApplicationDX11.h"

using namespace forward;

class InitDirect3DApp : public Application
{
public:
	InitDirect3DApp(HINSTANCE hInstance, int width, int height);
	~InitDirect3DApp();

protected:
	virtual void UpdateScene(float dt);
	virtual void DrawScene();
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
	: Application(hInstance, width, height)
{
}

InitDirect3DApp::~InitDirect3DApp()
{
}

void InitDirect3DApp::UpdateScene(float /*dt*/)
{

}

void InitDirect3DApp::DrawScene()
{
	m_pRender->pImmPipeline->ClearBuffers(Colors::Blue);
	m_pRender->Present(MainWnd(), 0);
}

void InitDirect3DApp::OnEsc()
{
	RequestTermination();
}