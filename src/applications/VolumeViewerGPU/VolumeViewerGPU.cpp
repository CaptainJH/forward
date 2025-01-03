#pragma warning( push )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#include <OpenVDB/openvdb.h>
#pragma warning( pop )

#include "Application.h"
#include "renderers/RasterGBufferRenderer.h"

using namespace forward;

class VolumeViewerGPU : public Application
{
	constexpr static u32 baseResolution = 128U;
	constexpr static u32 GridCount = 60U;
	struct CB
	{
		float4x4 ProjectionToWorld;
		float4   CameraPosition;
		u32		FrameCount;
		u32		Width;
		u32		Height;
		u32		VolumeIndex;
	};

public:
	VolumeViewerGPU(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Volume Rendering demo on GPU";
	}

    bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override { m_pauseRotate = !m_pauseRotate; }
	void OnEnter() override { m_pauseReplay = !m_pauseReplay; }

	void readGridFromVDB(u32 idx);

private:
	shared_ptr<Texture2D> m_uavRT;
	shared_ptr<ConstantBuffer<CB>> m_cb;
	shared_ptr<StructuredBuffer<f32>> m_gridBuffer[GridCount];
	std::unique_ptr<RenderPass> m_volumePass;
	std::vector<f32> m_grid[GridCount];
	u32 m_frames = 0U;
	bool m_pauseRotate = true;
	bool m_pauseReplay = false;
	f32 m_currentVolumeIdx = 0.0f;
};

void VolumeViewerGPU::UpdateScene(f32 dt)
{
	if (!m_pauseRotate)
	{
		const auto radiansToRotateBy = dt * 0.001f;
		float4x4 rotMat;
		rotMat.rotate(float3{ 0, radiansToRotateBy, 0 });
		auto eyePos3 = mFPCamera.GetPosition();
		float4 eyePos(eyePos3.x, eyePos3.y, eyePos3.z, 1.0f);
		eyePos = eyePos * rotMat;
		eyePos3 = { eyePos.x, eyePos.y, eyePos.z };
		mFPCamera.SetPosition(eyePos3);
		mFPCamera.LookAt(eyePos3, float3(0.0f), float3(0, 1, 0));
	}

	if (!m_pauseReplay)
	{
		m_currentVolumeIdx += dt * 0.01f;
		if (m_currentVolumeIdx > GridCount)
			m_currentVolumeIdx = 0.0f;
	}

	mFPCamera.UpdateViewMatrix();
	*m_cb = {
		.ProjectionToWorld = (mFPCamera.GetViewMatrix() * mFPCamera.GetProjectionMatrix()).inverse(),
		.CameraPosition = mFPCamera.GetPosition(),
		.FrameCount = m_frames++,
		.Width = static_cast<u32>(mClientWidth),
		.Height = static_cast<u32>(mClientHeight),
		.VolumeIndex = static_cast<u32>(std::floor(m_currentVolumeIdx)),
	};
}

void VolumeViewerGPU::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_volumePass.get());
	m_pDevice->DrawScreenText(GetFrameStats(), 10, 50, Colors::Red);
	m_pDevice->EndDrawFrameGraph();
}

void VolumeViewerGPU::readGridFromVDB(u32 idx)
{
	std::wstring vdbFilePath = FileSystem::getSingleton().GetSavedFolder() + L"smoke.vdb";
	openvdb::io::File vdbFile(TextHelper::ToAscii(vdbFilePath));
	vdbFile.open();
	std::stringstream gridName;
	gridName << "smoke." << idx + 1 << ".grid";
	auto gridIt = std::find_if(vdbFile.beginName(), vdbFile.endName(), [&](openvdb::Name n) {
		return n == gridName.str();
		});
	assert(gridIt != vdbFile.endName());
	m_grid[idx].resize(baseResolution * baseResolution * baseResolution, 0.0f);
	auto vdbGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(vdbFile.readGrid(gridIt.gridName()));
	for (openvdb::FloatGrid::ValueOnCIter iter = vdbGrid->cbeginValueOn(); iter; ++iter)
	{
		auto coord = iter.getCoord();
		m_grid[idx][(coord.z() * baseResolution + coord.y()) * baseResolution + coord.x()] = *iter;
	}
	vdbFile.close();
}

bool VolumeViewerGPU::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	float3 camPos = { 0, 20.0f, -50.0f };
	mFPCamera.SetPosition(camPos);
	mFPCamera.SetLens(AngleToRadians(65), AspectRatio(), 0.001f, 10000.0f);
	mFPCamera.LookAt(camPos, float3(0.0f), float3(0, 1, 0));
	openvdb::initialize();

	// forward::shared_ptr is not thread-safe, so they have to be initialized in the main thread.
	for (auto idx = 0U; idx < GridCount; ++idx)
	{
		constexpr u32 size = baseResolution * baseResolution * baseResolution;
		m_gridBuffer[idx] = make_shared<StructuredBuffer<f32>>("VolumeRendering_GridDataBuffer", size);
	}
	std::for_each(std::execution::par, std::begin(m_gridBuffer), std::end(m_gridBuffer), [&](auto& buf) {
		const u32 idx = static_cast<u32>(&buf - m_gridBuffer);
		readGridFromVDB(idx);
		for (auto i = 0U; i < m_grid[idx].size(); ++i)
			(*m_gridBuffer[idx])[i] = m_grid[idx][i];
		});

	m_volumePass = std::make_unique<RenderPass>([&](RenderPassBuilder& /*builder*/, ComputePipelineStateObject& pso) {

		m_cb = make_shared<ConstantBuffer<CB>>("Volume_CB");
		pso.m_CSState.m_constantBuffers[0] = m_cb;

		auto rt = m_pDevice->GetDefaultRT();
		m_uavRT = make_shared<Texture2D>("Volume_UAV_OUTPUT", DF_R8G8B8A8_UNORM, rt->GetWidth(), rt->GetHeight(), TextureBindPosition::TBP_Shader);
		m_uavRT->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);
		pso.m_CSState.m_uavShaderRes[0] = m_uavRT;
		for (auto i = 0U; i < GridCount; ++i)
			pso.m_CSState.m_shaderResources[i] = m_gridBuffer[i];

		// setup shaders
		pso.m_CSState.m_shader = make_shared<ComputeShader>("Volume_Shader", L"VolumeRendering", "VolumeMain");
		},
		[&](CommandList& cmdList) {
			const u32 x = mClientWidth / 8;
			const u32 y = mClientHeight / 8;
			cmdList.Dispatch(x, y, 1);
			cmdList.CopyResource(*m_pDevice->GetCurrentSwapChainRT(), *m_uavRT);
		});

	return true;
}

FORWARD_APPLICATION_MAIN(VolumeViewerGPU, 640, 480);
