//***************************************************************************************
// CommandList.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "types.h"
#include "RHI/ResourceSystem/DeviceResource.h"

namespace forward
{
	class Device;
	class FrameGraph;
	class RenderPass;
	class Texture2D;
	struct RTPipelineStateObject;

	class CommandList : public intrusive_ref_counter
	{
	public:
		CommandList(Device& device) : m_device(device) {}
		virtual ~CommandList() {}

		virtual void Reset() = 0;
		virtual void Close() = 0;
		Device& GetDevice() { return m_device; }

		virtual void DrawScreenText(const std::string& msg, i32 x, i32 y, const float4& color) = 0;
		virtual void Draw(u32 vertexNum, u32 startVertexLocation = 0) = 0;
		virtual void DrawIndexed(u32 indexCount, u32 baseVertex=0, u32 baseIndex=0) = 0;
		virtual void Dispatch(u32 x, u32 y, u32 z) = 0;
		virtual void DispatchRays(RTPipelineStateObject& pso) = 0;
		virtual void CopyResource(Resource& dst, Resource& src) = 0;
		virtual void ResolveResource(Texture2D* dst, Texture2D* src) = 0;

		// FrameGraph APIs
		virtual void BeginDrawFrameGraph(FrameGraph* fg) = 0;
		virtual void EndDrawFrameGraph() = 0;
		virtual void DrawRenderPass(RenderPass& pass) = 0;
		virtual void PopulateCmdsFrom(FrameGraph* fg) = 0;

	protected:
		Device& m_device;
	};
}
