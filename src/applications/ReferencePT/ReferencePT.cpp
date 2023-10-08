#include "Application.h"
#include "FrameGraph/Geometry.h"
#include "renderers/ReferencePTRenderer.h"
#include "dx12/DeviceDX12.h"
#include "dx12/CommandListDX12.h"
#include "dx12/CommandQueueDX12.h"
#include "dx12/ShaderSystem/ShaderDX12.h"
#include "dx12/ResourceSystem/DeviceBufferDX12.h"
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DevicePipelineStateObjectDX12.h"

using namespace forward;

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

		mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 100.0f);
		mFPCamera.SetPosition(float3(0.0f, 0.0f, -3.0f));
		m_pDeviceDX12 = static_cast<DeviceDX12*>(m_pDevice);
		m_scene = SceneData::LoadFromFile(L"DamagedHelmet/DamagedHelmet.gltf", m_pDevice->mLoadedResourceMgr);
		m_rtPSO = std::make_unique<RTPipelineStateObject>(m_scene);
		m_rtPSO->m_maxPayloadSizeInByte = 64;

		m_uav0Tex = make_shared<Texture2D>("UAV0_Tex", DF_R8G8B8A8_UNORM, mClientWidth, mClientHeight, TextureBindPosition::TBP_Shader);
		m_uav0Tex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		m_uav1Tex = make_shared<Texture2D>("UAV1_Tex", DF_R32G32B32A32_FLOAT, mClientWidth, mClientHeight, TextureBindPosition::TBP_Shader);
		m_uav1Tex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);

		m_materials = make_shared<StructuredBuffer<SceneData::MaterialData>>("MaterialDataBuffer", (u32)m_scene.mMaterials.size());
		for (auto i = 0U; i < m_scene.mMaterials.size(); ++i)
			(*m_materials)[i] = m_scene.mMaterials[i].materialData;

		m_cb = make_shared<ConstantBuffer<RaytracingData>>("g_sceneCB");

		prepareDeviceResources();

		// setup shaders
		m_rtPSO->m_rtState.m_shader = make_shared<RaytracingShaders>("RaytracingShader", L"PathTracer");

		m_rtPSO->m_rtState.m_constantBuffers[0] = m_cb;
		m_rtPSO->m_rtState.m_uavShaderRes[0] = m_uav0Tex;
		m_rtPSO->m_rtState.m_uavShaderRes[1] = m_uav1Tex;

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
		m_rtPSO->m_rtState.m_bindlessShaderStageStates.emplace_back(BindlessShaderStage{
			.m_space = ShaderDX12::MaterialDataSpace,
			.m_shaderResources = { m_materials },
			});
		m_rtPSO->m_rtState.m_samplers[0] = make_shared<SamplerState>("Sampler0");

		m_rtPSO->m_rtState.m_rayGenShaderTable = forward::make_shared<ShaderTable>("RayGenShaderTable", Vector<WString>{ L"RayGen" });
		m_rtPSO->m_rtState.m_hitShaderTable = forward::make_shared<ShaderTable>("HitGroupShaderTable",
			Vector<WString>{ L"HitGroup_ClosestHit", L"HitGroup_AnyHit", L"HitGroupShadow_AnyHitShadow" });
		m_rtPSO->m_rtState.m_missShaderTable = forward::make_shared<ShaderTable>("MissShaderTable", 
			Vector<WString>{ L"Miss", L"MissShadow" });

		m_rtPSO->m_devicePSO = make_shared<DeviceRTPipelineStateObjectDX12>(m_pDeviceDX12, *m_rtPSO);

		return true;
	}

protected:
	void UpdateScene(f32) override
	{
		mFPCamera.UpdateViewMatrix();
		static u32 frames = 0;
		++frames;

		auto viewMat = mFPCamera.GetViewMatrix();
		static float4x4 lastViewMat = viewMat;
		static u32 accumulatedFrames = 0;
		bool resetAccumulation = !lastViewMat.equalWithAbsError(viewMat, std::numeric_limits<f32>::epsilon());
		if (resetAccumulation)
			accumulatedFrames = 1;
		else
			++accumulatedFrames;
		
		*m_cb = RaytracingData{
			.view = viewMat.inverse(),
			.proj = mFPCamera.GetProjectionMatrix(),

			.skyIntensity = 3.0f,
			.lightCount = 0,
			.frameNumber = frames,
			.maxBounces = 8,

			.exposureAdjustment = 0.2f,
			.accumulatedFrames = accumulatedFrames,
			.enableAntiAliasing = TRUE,
			.focusDistance = 10.0f,

			.apertureSize = 0.0f,
			.enableAccumulation = resetAccumulation ? FALSE : TRUE,

			.lights = {}
		};

		lastViewMat = viewMat;
	}

	void DrawScene() override
	{
		m_pDeviceDX12->BeginDraw();

		if (auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12())
		{
			auto devicePSO = dynamic_cast<DeviceRTPipelineStateObjectDX12*>(m_rtPSO->m_devicePSO.get());
			cmdList->SetDynamicConstantBuffer(m_cb.get());
			cmdList->BindGPUVisibleHeaps(*devicePSO);
			cmdList->BindRTPSO(*devicePSO);
			cmdList->DispatchRays(*m_rtPSO);
			cmdList->CopyResource(*m_pDeviceDX12->GetCurrentSwapChainRT(), *m_uav0Tex);
		}

		m_pDeviceDX12->EndDraw();
	}

	std::unique_ptr<RTPipelineStateObject> m_rtPSO;
	shared_ptr<ConstantBuffer<RaytracingData>> m_cb;
	shared_ptr<Texture2D> m_uav0Tex;
	shared_ptr<Texture2D> m_uav1Tex;
	shared_ptr<StructuredBuffer<SceneData::MaterialData>> m_materials;

	DeviceDX12* m_pDeviceDX12 = nullptr;
	SceneData m_scene;

private:
	void prepareDeviceResources()
	{
		auto cmdList = m_pDeviceDX12->GetDefaultQueue()->GetCommandListDX12();
		auto cmdListDevice = cmdList->GetDeviceCmdListPtr();
		cmdList->SetDynamicConstantBuffer(m_cb.get());
		for (auto& gp : m_rtPSO->m_meshes)
		{
			gp.first->SetDeviceObject(make_shared<DeviceBufferDX12>(cmdListDevice.Get(), gp.first.get(), *m_pDeviceDX12));
			gp.second->SetDeviceObject(make_shared<DeviceBufferDX12>(cmdListDevice.Get(), gp.second.get(), *m_pDeviceDX12));
		}

		m_uav0Tex->SetDeviceObject(make_shared<DeviceTexture2DDX12>(m_uav0Tex.get(), *m_pDeviceDX12));
		m_uav1Tex->SetDeviceObject(make_shared<DeviceTexture2DDX12>(m_uav1Tex.get(), *m_pDeviceDX12));
		m_materials->SetDeviceObject(make_shared<DeviceBufferDX12>(cmdListDevice.Get(), m_materials.get(), *m_pDeviceDX12));

		for (auto& tex : m_scene.mTextures)
			tex->SetDeviceObject(make_shared<DeviceTexture2DDX12>(tex.get(), *m_pDeviceDX12));
	}
};

FORWARD_APPLICATION_MAIN(ReferencePT, 1920, 1080);