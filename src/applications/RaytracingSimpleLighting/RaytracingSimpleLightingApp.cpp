#include "Application.h"
#include "FrameGraph/Geometry.h"
#include "dx12/DeviceDX12.h"
#include "dx12/CommandListDX12.h"
#include "dx12/CommandQueueDX12.h"
#include "dx12/ShaderSystem/ShaderDX12.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DevicePipelineStateObjectDX12.h"
#include "SceneData.h"

using namespace forward;

struct SceneConstantBuffer
{
	float4x4 projectionToWorld;
	float4 cameraPosition;
	float4 lightPosition;
	float4 lightAmbientColor;
	float4 lightDiffuseColor;
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

		m_pDeviceDX12 = static_cast<DeviceDX12*>(m_pDevice);
		m_scene = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);
		m_rtPSO = std::make_unique<RTPipelineStateObject>(m_scene);

		m_uavTex = make_shared<Texture2D>("UAV_Tex", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_Shader);
		m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		m_rtPSO->m_rtState.m_uavShaderRes[0] = m_uavTex;

		m_cb0 = make_shared<ConstantBuffer<SceneConstantBuffer>>("g_sceneCB");
		m_cb1 = make_shared<ConstantBuffer<CubeConstantBuffer>>("g_cubeCB");

		*m_cb1 = {
			.albedo = {1.0f, 1.0f, 1.0f, 1.0f}
		};
		prepareDeviceResources();

		m_rtPSO->m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"RaytracingSimpleLighting");

		m_rtPSO->m_rtState.m_constantBuffers[0] = m_cb0;
		m_rtPSO->m_rtState.m_constantBuffers[1] = m_cb1;
		m_rtPSO->m_rtState.m_uavShaderRes[0] = m_uavTex;

		auto& newlyAddedStageVB = m_rtPSO->m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
			.m_space = ShaderDX12::VertexDataSpace });
		for (auto& m : m_scene.mMeshData)
			newlyAddedStageVB.m_shaderResources.emplace_back(m.m_VB);
		auto& newlyAddedStageIB = m_rtPSO->m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
			.m_space = ShaderDX12::IndexDataSpace });
		for (auto& m : m_scene.mMeshData)
			newlyAddedStageIB.m_shaderResources.emplace_back(m.m_IB);

		m_rtPSO->m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RayGenShaderTable", L"MyRaygenShader");
		m_rtPSO->m_rtState.m_hitShaderTable = make_shared<ShaderTable>("HitGroupShaderTable", L"HitGroup_MyClosestHitShader");
		m_rtPSO->m_rtState.m_missShaderTable = make_shared<ShaderTable>("MissShaderTable", L"MyMissShader");

		m_rtPSO->m_devicePSO = make_shared<DeviceRTPipelineStateObjectDX12>(m_pDeviceDX12, *m_rtPSO);

		return true;
	}

protected:
	void UpdateScene(f32 dt) override
	{
		const auto radiansToRotateBy = dt * 0.001f;
		float4x4 rotMat;
		rotMat.rotate(float3{ 0, radiansToRotateBy, 0 });
		float4 eyePos(m_eyePos.x, m_eyePos.y, m_eyePos.z, 1.0f);
		eyePos = eyePos * rotMat;
		m_eyePos = { eyePos.x, eyePos.y, eyePos.z };

		const auto view = ToFloat4x4(Matrix4f::LookAtLHMatrix(m_eyePos, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));
		const auto proj = ToFloat4x4(Matrix4f::PerspectiveFovLHMatrix(0.25f * f_PI, AspectRatio(), 1.0f, 125.0f));
		const float4x4 viewProj =   view * proj;
		
		*m_cb0 = SceneConstantBuffer{
			.projectionToWorld = viewProj.inverse(),
			.cameraPosition = eyePos,
			.lightPosition = {0.0f, 1.8f, -3.0f, 0.0f},
			.lightAmbientColor = {0.5f, 0.5f, 0.5f, 1.0f},
			.lightDiffuseColor = {0.5f, 0.0f, 0.0f, 1.0f}
		};
	}

	void DrawScene() override
	{
		m_pDeviceDX12->BeginDraw();

		if (auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12())
		{
			auto& devicePSO = *dynamic_cast<DeviceRTPipelineStateObjectDX12*>(m_rtPSO->m_devicePSO.get());
			cmdList->SetDynamicConstantBuffer(m_cb0.get());
			cmdList->BindGPUVisibleHeaps(devicePSO);
			cmdList->BindRTPSO(devicePSO);
			cmdList->DispatchRays(*m_rtPSO);
			cmdList->CopyResource(*m_pDeviceDX12->GetCurrentSwapChainRT(), *m_uavTex);
		}

		m_pDeviceDX12->EndDraw();
	}

	std::unique_ptr<RTPipelineStateObject> m_rtPSO;
	shared_ptr<ConstantBuffer<SceneConstantBuffer>> m_cb0;
	shared_ptr<ConstantBuffer<CubeConstantBuffer>> m_cb1;
	shared_ptr<Texture2D> m_uavTex;
	DeviceDX12* m_pDeviceDX12 = nullptr;

	Vector3f m_eyePos = { 0.0f, 2.0f, -5.0f };

	SceneData m_scene;

private:
	void prepareDeviceResources()
	{
		auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12();
		auto commandList = cmdList->GetDeviceCmdListPtr().Get();
		cmdList->SetDynamicConstantBuffer(m_cb0.get());
		cmdList->SetDynamicConstantBuffer(m_cb1.get());
		for (auto& gp : m_rtPSO->m_meshes)
		{
			gp.first->SetDeviceObject(make_shared<DeviceBufferDX12>(commandList, gp.first.get(), *m_pDeviceDX12));
			gp.second->SetDeviceObject(make_shared<DeviceBufferDX12>(commandList, gp.second.get(), *m_pDeviceDX12));
		}

		m_uavTex->SetDeviceObject(make_shared<DeviceTexture2DDX12>(m_uavTex.get(), *m_pDeviceDX12));
	}
};

FORWARD_APPLICATION_MAIN(RaytracingSimpleLighting, 1920, 1080);