
#include "Application.h"
#include "FrameGraph/Geometry.h"
#include "dx12/DeviceDX12.h"
#include "dx12/CommandListDX12.h"
#include "dx12/CommandQueueDX12.h"
#include "dx12/ShaderSystem/ShaderDX12.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DevicePipelineStateObjectDX12.h"

using namespace forward;

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};

class HelloRaytracing : public Application
{
public:
	HelloRaytracing(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Hello Raytracing!";
	}

	~HelloRaytracing()
	{
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!Application::Init())
			return false;

		m_rayGenCB.viewport = { -1, -1, 1, 1 };
		const f32 border = 0.1f;
		m_rayGenCB.stencil = {
			-1 + border / AspectRatio(), -1 + border,
			 1 - border / AspectRatio(), 1.0f - border
		};

		m_pDeviceDX12 = static_cast<DeviceDX12*>(m_pDevice);
		auto commandList = m_pDeviceDX12->DeviceCommandList();
		m_rtPSO = std::make_unique<RTPipelineStateObject>();

		// setup geometry
		m_ib = make_shared<IndexBuffer>("IndexBuffer", PT_TRIANGLELIST, 3);
		m_ib->AddFace(TriangleIndices(0, 1, 2));
		auto deviceIB = make_shared<DeviceBufferDX12>(commandList, m_ib.get(), *m_pDeviceDX12);
		m_ib->SetDeviceObject(deviceIB);

		f32 depthValue = 1.0;
		f32 offset = 0.7f;
		Vertex_POS vertices[] =
		{
			// The sample raytraces in screen space coordinates.
			// Since DirectX screen space coordinates are right handed (i.e. Y axis points down).
			// Define the vertices in counter clockwise order ~ clockwise in left handed.
			Vector3f { 0, -offset, depthValue },
			Vector3f { -offset, offset, depthValue },
			Vector3f { offset, offset, depthValue }
		};
		m_vb = make_shared<VertexBuffer>("VertexBuffer", Vertex_POS::GetVertexFormat(), 3);
		m_vb->AddVertex(vertices[0]);
		m_vb->AddVertex(vertices[1]);
		m_vb->AddVertex(vertices[2]);
		auto deviceVB = make_shared<DeviceBufferDX12>(commandList, m_vb.get(), *m_pDeviceDX12);
		m_vb->SetDeviceObject(deviceVB);
		m_rtPSO->m_meshes.emplace_back(std::make_pair(m_vb, m_ib));

		m_cb = make_shared<ConstantBuffer<RayGenConstantBuffer>>("g_sceneCB");
		*m_cb = m_rayGenCB;
		m_rtPSO->m_rtState.m_constantBuffers[0] = m_cb;

		m_uavTex = make_shared<Texture2D>("UAV_Tex", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_Shader);
		m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		auto deviceUAVTex = make_shared<DeviceTexture2DDX12>(m_uavTex.get(), *m_pDeviceDX12);
		m_uavTex->SetDeviceObject(deviceUAVTex);
		m_rtPSO->m_rtState.m_uavShaderRes[0] = m_uavTex;

		// setup shaders
		m_rtPSO->m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"HelloRaytracing");
		m_rtPSO->m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RayGenShaderTable", L"MyRaygenShader");
		m_rtPSO->m_rtState.m_hitShaderTable = make_shared<ShaderTable>("HitGroupShaderTable", L"HitGroup_MyClosestHitShader");
		m_rtPSO->m_rtState.m_missShaderTable = make_shared<ShaderTable>("MissShaderTable", L"MyMissShader");

		m_rtPSO->m_devicePSO = make_shared<DeviceRTPipelineStateObjectDX12>(m_pDeviceDX12, *m_rtPSO);		

		return true;
	}

protected:
	void UpdateScene(f32) override
	{

	}

	void DrawScene() override
	{
		m_pDeviceDX12->BeginDraw();

		auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12();
		cmdList->BindGPUVisibleHeaps();
		cmdList->PrepareGPUVisibleHeaps(*m_rtPSO);
		cmdList->CommitStagedDescriptors();
		cmdList->BindRTPSO(*dynamic_cast<DeviceRTPipelineStateObjectDX12*>(m_rtPSO->m_devicePSO.get()));
		cmdList->DispatchRays(*m_rtPSO);
		cmdList->CopyResource(*m_pDeviceDX12->GetCurrentSwapChainRT(), *m_uavTex);

		m_pDeviceDX12->EndDraw();
	}

	std::unique_ptr<RTPipelineStateObject> m_rtPSO;
	shared_ptr<IndexBuffer> m_ib;
	shared_ptr<VertexBuffer> m_vb;
	shared_ptr<ConstantBuffer<RayGenConstantBuffer>> m_cb;
	shared_ptr<Texture2D> m_uavTex;
	DeviceDX12* m_pDeviceDX12 = nullptr;

	// Raytracing scene
	RayGenConstantBuffer m_rayGenCB;
};

FORWARD_APPLICATION_MAIN(HelloRaytracing, 1920, 1080);