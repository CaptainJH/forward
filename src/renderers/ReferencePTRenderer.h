#pragma once
#include "FrameGraph/Renderer.h"
#include "RHI/SceneData.h"
#include "SceneData.h"

namespace forward
{
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
		BOOL enableAntiAliasing;
		f32 focusDistance;

		f32 apertureSize;
		BOOL enableAccumulation;
		f32 pad;
		f32 pad2;

		Light lights[4];
	};


	class ReferencePTRenderer final : public RendererBase
	{
	public:
		ReferencePTRenderer(SceneData& sd)
		{
			sd;
		}


		shared_ptr<ConstantBuffer<RaytracingData>> m_cb;
		shared_ptr<StructuredBuffer<SceneData::MaterialData>> m_materials;

		void SetupRenderPass(Device& d)
		{
			m_renderPassVec.emplace_back(RenderPass(
				[&](RenderPassBuilder& /*builder*/, RTPipelineStateObject& pso) {
					pso;
					d;
				},
				[&](CommandList& cmdList) {
					cmdList;
					//r.DrawIndexed(p.second->GetNumElements());
				}
			));
		}
	};

}