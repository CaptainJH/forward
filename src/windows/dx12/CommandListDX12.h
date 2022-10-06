//***************************************************************************************
// CommandListDX12.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "RHI/CommandList.h"
#include "windows/dx12/dx12Util.h"
#include "DynamicDescriptorHeapDX12.h"

namespace forward
{
	class CommandQueueDX12;

	class CommandListDX12 final : public CommandList
	{
	public:
		virtual ~CommandListDX12();
		void Reset() override;
		void Close() override;

		void DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color) override;
		void Draw(u32 vertexNum, u32 startVertexLocation) override;
		void DrawIndexed(u32 indexCount) override;

		void BeginDrawFrameGraph(FrameGraph* fg) override;
		void EndDrawFrameGraph() override;
		void DrawRenderPass(RenderPass& pass) override;

	private:
		CommandListDX12(Device& d);
		CommandAllocatorComPtr				m_CmdListAlloc;
		CommandListComPtr						m_CmdList;

		DynamicDescriptorHeapDX12			m_DynamicDescriptorHeaps[2] =
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		};

		friend class CommandQueueDX12;
	};
}
