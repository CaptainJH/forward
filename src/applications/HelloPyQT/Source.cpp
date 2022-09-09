#include <SDKDDKVer.h>
#include <iostream>
#include "HelloPyQT.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"

using namespace forward;

class HelloPyQTDll : public Application
{
public:
	HelloPyQTDll(HWND hwnd, i32 width, i32 height)
		: Application(hwnd, width, height)
	{}

	~HelloPyQTDll()
	{}

	bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

private:

	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	shared_ptr<ConstantBuffer<Matrix4f>> m_constantBuffer;

	std::unique_ptr<RenderPass> m_renderPass;
	std::unique_ptr<SimpleGeometry> m_geometry;

};

bool HelloPyQTDll::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	m_renderPass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder& builder, PipelineStateObject& pso) {

		// Build the view matrix.
		Vector3f pos = Vector3f(0.0f, 1.0f, -5.0f);
		Vector3f target; target.MakeZero();
		Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
		m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
		// Build the projection matrix
		m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * Pi, AspectRatio(), 0.01f, 100.0f);

		// setup shaders
		pso.m_VSState.m_shader = forward::make_shared<VertexShader>("HelloFrameGraphVS", L"BasicShader.hlsl", L"VSMain");
		pso.m_PSState.m_shader = forward::make_shared<PixelShader>("HelloFrameGraphPS", L"BasicShader.hlsl", L"PSMain");

		// setup geometry
		m_geometry = std::make_unique<SimpleGeometry>("BOX", forward::GeometryBuilder<forward::GP_COLOR_BOX>());
		builder << *m_geometry;

		// setup constant buffer
		m_constantBuffer = make_shared<ConstantBuffer<Matrix4f>>("CB");
		pso.m_VSState.m_constantBuffers[0] = m_constantBuffer;

		// setup render states
		auto dsPtr = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultDS");
		pso.m_OMState.m_depthStencilResource = dsPtr;

		auto rsPtr = GraphicsObject::FindFrameGraphObject<Texture2D>("DefaultRT");
		pso.m_OMState.m_renderTargetResources[0] = rsPtr;
	},
		[&](Device& render) {
		render.DrawIndexed(m_geometry->GetIndexCount());
	});

	return true;
}

void HelloPyQTDll::UpdateScene(f32 /*dt*/)
{
	auto frames = (f32)mTimer.FrameCount() / 100;
	auto worldMat = Matrix4f::RotationMatrixY(frames) * Matrix4f::RotationMatrixX(frames);
	*m_constantBuffer = worldMat * m_viewMat * m_projMat;
}

void HelloPyQTDll::DrawScene()
{
	m_pDevice->DrawRenderPass(*m_renderPass);
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/,
	DWORD  ul_reason_for_call,
	LPVOID /*lpReserved*/
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HelloPyQTDll* gApp = nullptr;

void Forward_Constructor(HWND hwnd, int w, int h)
{
	if (!gApp)
	{
		//::MessageBoxA(hwnd, "Boom", "Boom", MB_OK);

		gApp = new HelloPyQTDll(hwnd, w, h);

		if (!gApp->Init())
			return;
	}
}

void Forward_Update()
{
	if (gApp)
	{
		gApp->Run();
	}
}

void Forward_Destructor()
{
	SAFE_DELETE(gApp);
}