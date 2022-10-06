//***************************************************************************************
// CommandList.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "Types.h"
#include "DataFormat.h"
#include "Vector4f.h"
#include "ResourceSystem/DeviceResource.h"

namespace forward
{
	class Device;
	class FrameGraph;
	class RenderPass;

	class CommandList : public intrusive_ref_counter
	{
	public:
		CommandList(Device& device) : m_device(device) {}
		virtual ~CommandList() {}

		virtual void Reset() = 0;
		virtual void Close() = 0;
		Device& GetDevice() { return m_device; }

		virtual void DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color) = 0;
		virtual void Draw(u32 vertexNum, u32 startVertexLocation = 0) = 0;
		virtual void DrawIndexed(u32 indexCount) = 0;

		// FrameGraph APIs
		virtual void BeginDrawFrameGraph(FrameGraph* fg) = 0;
		virtual void EndDrawFrameGraph() = 0;
		virtual void DrawRenderPass(RenderPass& pass) = 0;

	protected:
		Device& m_device;
	};
}
