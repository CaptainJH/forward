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

struct CubeConstantBuffer
{
	Vector4f albedo;
};

struct Light {
	float3 position;
	u32 type;
	float3 intensity;
	u32 pad;
};

struct RaytracingData
{
	float4x4 view;
	float4x4 proj;

	f32 skyIntensity;
	u32 lightCount;
	u32 frameNumber;
	u32 maxBounces;

	f32 exposureAdjustment;
	u32 accumulatedFrames;
	bool enableAntiAliasing;
	f32 focusDistance;

	f32 apertureSize;
	bool enableAccumulation;
	f32 pad;
	f32 pad2;

	Light lights[4];
};


class ReferencePT : public Application
{
public:
	ReferencePT(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Reference Path Tracer";
	}

	~ReferencePT()
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

		m_uav0Tex = make_shared<Texture2D>("UAV0_Tex", forward::DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, forward::TextureBindPosition::TBP_Shader);
		m_uav0Tex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		m_uav1Tex = make_shared<Texture2D>("UAV1_Tex", forward::DF_R32G32B32A32_FLOAT, mClientWidth, mClientHeight, forward::TextureBindPosition::TBP_Shader);
		m_uav1Tex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);

		prepareDeviceResources();

		// setup shaders
		m_rtPSO->m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"PathTracer");
		m_cb = make_shared<ConstantBuffer<RaytracingData>>("g_sceneCB");

		m_rtPSO->m_rtState.m_constantBuffers[0] = m_cb;
		m_rtPSO->m_rtState.m_uavShaderRes[0] = m_uav0Tex;
		m_rtPSO->m_rtState.m_uavShaderRes[1] = m_uav1Tex;
		m_rtPSO->m_rtState.m_rayGenShaderTable = make_shared<ShaderTable>("RayGenShaderTable", 1U, 0U);
		m_rtPSO->m_rtState.m_rayGenShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"RayGen" });
		m_rtPSO->m_rtState.m_hitShaderTable = make_shared<ShaderTable>("HitGroupShaderTable", 1U, 0U);
		m_rtPSO->m_rtState.m_hitShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"ClosestHit" });
		m_rtPSO->m_rtState.m_hitShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"AnyHit" });
		m_rtPSO->m_rtState.m_hitShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"AnyHitShadow" });
		m_rtPSO->m_rtState.m_missShaderTable = make_shared<ShaderTable>("MissShaderTable", 1U, 0U);
		m_rtPSO->m_rtState.m_missShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"Miss" });
		m_rtPSO->m_rtState.m_missShaderTable->m_shaderRecords.emplace_back(ShaderRecordDesc{ L"MissShadow" });

		auto& newlyAddedStageVB = m_rtPSO->m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{ 
			.m_space = ShaderDX12::VertexDataSpace });
		for (auto& m : m_scene.mMeshData)
			newlyAddedStageVB.m_shaderResources.emplace_back(m.m_VB);
		auto& newlyAddedStageIB = m_rtPSO->m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
			.m_space = ShaderDX12::IndexDataSpace });
		for (auto& m : m_scene.mMeshData)
			newlyAddedStageIB.m_shaderResources.emplace_back(m.m_IB);
		auto& newlyAddedStageTex = m_rtPSO->m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
			.m_space = ShaderDX12::TextureSpace });
		for (auto& t : m_scene.mTextures)
			newlyAddedStageTex.m_shaderResources.emplace_back(t);

		m_rtPSO->m_deviceRTPSO = forward::make_shared<DeviceRTPipelineStateObjectDX12>(m_pDeviceDX12, *m_rtPSO);

		return true;
	}

protected:
	void UpdateScene(f32 dt) override
	{
		const auto radiansToRotateBy = dt * 0.001f;
		const Matrix4f rotMat = Matrix4f::RotationMatrixY(radiansToRotateBy);
		Vector4f eyePos(m_eyePos, 1.0f);
		eyePos = rotMat * eyePos;
		m_eyePos = eyePos.xyz();

		const auto view = Matrix4f::LookAtLHMatrix(m_eyePos, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		const Matrix4f proj = Matrix4f::PerspectiveFovLHMatrix(0.25f * Pi, AspectRatio(), 1.0f, 125.0f);
		const Matrix4f viewProj = view * proj;
		
		//*m_cb = SceneConstantBuffer{
		//	.projectionToWorld = viewProj.Inverse(),
		//	.cameraPosition = eyePos,
		//	.lightPosition = {0.0f, 1.8f, -3.0f, 0.0f},
		//	.lightAmbientColor = {0.5f, 0.5f, 0.5f, 1.0f},
		//	.lightDiffuseColor = {0.5f, 0.0f, 0.0f, 1.0f}
		//};
	}

	void DrawScene() override
	{
		m_pDeviceDX12->BeginDraw();

		auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12();
		cmdList->BindGPUVisibleHeaps();
		cmdList->PrepareGPUVisibleHeaps(*m_rtPSO);
		cmdList->CommitStagedDescriptors();
		cmdList->BindRTPSO(*dynamic_cast<DeviceRTPipelineStateObjectDX12*>(m_rtPSO->m_deviceRTPSO.get()));
		cmdList->DispatchRays(*m_rtPSO);
		//cmdList->CopyResource(*m_pDeviceDX12->GetCurrentSwapChainRT(), *m_uavTex);

		m_pDeviceDX12->EndDraw();
	}

	std::unique_ptr<RTPipelineStateObject> m_rtPSO;
	shared_ptr<ConstantBuffer<RaytracingData>> m_cb;
	forward::shared_ptr<Texture2D> m_uav0Tex;
	forward::shared_ptr<Texture2D> m_uav1Tex;
	DeviceDX12* m_pDeviceDX12 = nullptr;

	Vector3f m_eyePos = Vector3f(0.0f, 2.0f, -5.0f);

	SceneData m_scene;

private:
	void prepareDeviceResources()
	{
		auto commandList = m_pDeviceDX12->DeviceCommandList();
		for (auto& gp : m_rtPSO->m_meshes)
		{
			gp.first->SetDeviceObject(forward::make_shared<DeviceBufferDX12>(commandList, gp.first.get(), *m_pDeviceDX12));
			gp.second->SetDeviceObject(forward::make_shared<DeviceBufferDX12>(commandList, gp.second.get(), *m_pDeviceDX12));
		}

		auto deviceUAVTex = forward::make_shared<DeviceTexture2DDX12>(m_uav0Tex.get(), *m_pDeviceDX12);
		m_uav0Tex->SetDeviceObject(deviceUAVTex);

		deviceUAVTex = forward::make_shared<DeviceTexture2DDX12>(m_uav1Tex.get(), *m_pDeviceDX12);
		m_uav1Tex->SetDeviceObject(deviceUAVTex);
	}


};

FORWARD_APPLICATION_MAIN(ReferencePT, 1920, 1080);