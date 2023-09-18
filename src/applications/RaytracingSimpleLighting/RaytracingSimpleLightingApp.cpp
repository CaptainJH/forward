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

struct SceneConstantBuffer
{
	Matrix4f projectionToWorld;
	Vector4f cameraPosition;
	Vector4f lightPosition;
	Vector4f lightAmbientColor;
	Vector4f lightDiffuseColor;
};

struct CubeConstantBuffer
{
	Vector4f albedo;
};


class RaytracingSimpleLighting : public Application
{
public:
	RaytracingSimpleLighting(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Raytracing with simple lighting!";
	}

	~RaytracingSimpleLighting()
	{
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!Application::Init())
			return false;

		//m_rayGenCB.viewport = { -1, -1, 1, 1 };
		//const f32 border = 0.1f;
		//m_rayGenCB.stencil = {
		//	-1 + border / AspectRatio(), -1 + border,
		//	 1 - border / AspectRatio(), 1.0f - border
		//};

		m_pDeviceDX12 = static_cast<DeviceDX12*>(m_pDevice);
		auto commandList = m_pDeviceDX12->DeviceCommandList();
		m_rtPSO = std::make_unique<RTPipelineStateObject>();

		// setup geometry
		m_ib = make_shared<IndexBuffer>("IndexBuffer", PT_TRIANGLELIST, 3 * 12);
		m_ib->AddFace({ 3, 1, 0 });
		m_ib->AddFace({ 2, 1, 3 });

		m_ib->AddFace({ 6, 4, 5 });
		m_ib->AddFace({ 7, 4, 6 });

		m_ib->AddFace({ 11, 9, 8 });
		m_ib->AddFace({ 10, 9, 11 });

		m_ib->AddFace({ 14, 12, 13 });
		m_ib->AddFace({ 15, 12, 14 });

		m_ib->AddFace({ 19, 17, 16 });
		m_ib->AddFace({ 18, 17, 19 });

		m_ib->AddFace({ 22, 20, 21 });
		m_ib->AddFace({ 23, 20, 22 });
		auto deviceIB = forward::make_shared<DeviceBufferDX12>(commandList, m_ib.get(), *m_pDeviceDX12);
		m_ib->SetDeviceObject(deviceIB);

		Vertex_POS_N vertices[] =
		{
			{ Vector3f(-1.0f, 1.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f) },
			{ Vector3f(1.0f, 1.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f) },
			{ Vector3f(1.0f, 1.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f) },
			{ Vector3f(-1.0f, 1.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f) },

			{ Vector3f(-1.0f, -1.0f, -1.0f), Vector3f(0.0f, -1.0f, 0.0f) },
			{ Vector3f(1.0f, -1.0f, -1.0f), Vector3f(0.0f, -1.0f, 0.0f) },
			{ Vector3f(1.0f, -1.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f) },
			{ Vector3f(-1.0f, -1.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f) },

			{ Vector3f(-1.0f, -1.0f, 1.0f), Vector3f(-1.0f, 0.0f, 0.0f) },
			{ Vector3f(-1.0f, -1.0f, -1.0f), Vector3f(-1.0f, 0.0f, 0.0f) },
			{ Vector3f(-1.0f, 1.0f, -1.0f), Vector3f(-1.0f, 0.0f, 0.0f) },
			{ Vector3f(-1.0f, 1.0f, 1.0f), Vector3f(-1.0f, 0.0f, 0.0f) },

			{ Vector3f(1.0f, -1.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f) },
			{ Vector3f(1.0f, -1.0f, -1.0f), Vector3f(1.0f, 0.0f, 0.0f) },
			{ Vector3f(1.0f, 1.0f, -1.0f), Vector3f(1.0f, 0.0f, 0.0f) },
			{ Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f) },

			{ Vector3f(-1.0f, -1.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f) },
			{ Vector3f(1.0f, -1.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f) },
			{ Vector3f(1.0f, 1.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f) },
			{ Vector3f(-1.0f, 1.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f) },

			{ Vector3f(-1.0f, -1.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f) },
			{ Vector3f(1.0f, -1.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f) },
			{ Vector3f(1.0f, 1.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f) },
			{ Vector3f(-1.0f, 1.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f) },
		};
		m_vb = make_shared<VertexBuffer>("VertexBuffer", Vertex_POS_N::GetVertexFormat(), 4 * 6);
		for (const auto& v : vertices)
			m_vb->AddVertex(v);
		auto deviceVB = forward::make_shared<DeviceBufferDX12>(commandList, m_vb.get(), *m_pDeviceDX12);
		m_vb->SetDeviceObject(deviceVB);
		m_rtPSO->m_geometry.emplace_back(std::make_pair(m_vb, m_ib));

		m_uavTex = make_shared<Texture2D>("UAV_Tex", forward::DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, forward::TextureBindPosition::TBP_Shader);
		m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		auto deviceUAVTex = forward::make_shared<DeviceTexture2DDX12>(m_uavTex.get(), *m_pDeviceDX12);
		m_uavTex->SetDeviceObject(deviceUAVTex);
		m_rtPSO->m_rtState.m_uavShaderRes[0] = m_uavTex;

		// setup shaders
		m_rtPSO->m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"RaytracingSimpleLighting");
		//m_rtPSO->m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RayGenShaderTable", 1U,
		//	static_cast<u32>(sizeof(RayGenConstantBuffer)));
		//Vector<u8> rayGenCBBuffer(sizeof(RayGenConstantBuffer), 0);
		//memcpy(rayGenCBBuffer.data(), &m_rayGenCB, rayGenCBBuffer.size());
		//m_rtPSO->m_rtState.m_rayGenShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"MyRaygenShader", rayGenCBBuffer });
		//m_rtPSO->m_rtState.m_hitShaderTable = make_shared<ShaderTable>("HitGroupShaderTable", 1U, 0U);
		//m_rtPSO->m_rtState.m_hitShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"MyClosestHitShader" });
		//m_rtPSO->m_rtState.m_missShaderTable = make_shared<ShaderTable>("MissShaderTable", 1U, 0U);
		//m_rtPSO->m_rtState.m_missShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"MyMissShader" });

		// Execute the initialization commands
		m_pDeviceDX12->GetDefaultQueue()->ExecuteCommandList([]() {});
		m_pDeviceDX12->GetDefaultQueue()->Flush();

		m_rtPSO->m_deviceRTPSO = forward::make_shared<DeviceRTPipelineStateObjectDX12>(m_pDeviceDX12, *m_rtPSO);

		return true;
	}

protected:
	void UpdateScene(f32) override
	{

	}

	void DrawScene() override
	{
		//m_pDeviceDX12->BeginDraw();

		//auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12();
		//cmdList->BindGPUVisibleHeaps();
		//cmdList->PrepareGPUVisibleHeaps(*m_rtPSO);
		//cmdList->CommitStagedDescriptors();
		//cmdList->BindRTPSO(*dynamic_cast<DeviceRTPipelineStateObjectDX12*>(m_rtPSO->m_deviceRTPSO.get()));
		//cmdList->DispatchRays(*m_rtPSO);
		//cmdList->CopyResource(*m_pDeviceDX12->GetCurrentSwapChainRT(), *m_uavTex);

		//m_pDeviceDX12->EndDraw();
	}

	std::unique_ptr<RTPipelineStateObject> m_rtPSO;
	shared_ptr<IndexBuffer> m_ib;
	shared_ptr<VertexBuffer> m_vb;
	forward::shared_ptr<Texture2D> m_uavTex;
	DeviceDX12* m_pDeviceDX12 = nullptr;

	// Raytracing scene
	//RayGenConstantBuffer m_rayGenCB;
};

FORWARD_APPLICATION_MAIN(RaytracingSimpleLighting, 1920, 1080);