//***************************************************************************************
// CommandListDX12.h by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "RHI/CommandList.h"
#include "windows/dx12/dx12Util.h"
#include "DynamicDescriptorHeapDX12.h"

namespace forward
{
	enum class QueueType;
	class DeviceDX12;
	class CommandQueueDX12;
	class DeviceResourceDX12;
	class DevicePipelineStateObjectDX12;
	class DeviceRTPipelineStateObjectDX12;

	class CommandListDX12 final : public CommandList
	{
	public:
		virtual ~CommandListDX12();
		void Reset() override;
		void Close() override;

		void DrawScreenText(const std::string& msg, i32 x, i32 y, const float4& color) override;
		void Draw(u32 vertexNum, u32 startVertexLocation) override;
		void DrawIndexed(u32 indexCount) override;
		void Dispatch(u32 x, u32 y, u32 z) override;
		void DispatchRays(RTPipelineStateObject& pso) override;
		void CopyResource(Resource& dst, Resource& src) override;
		void ResolveResource(Texture2D* dst, Texture2D* src) override;

		void BindRasterPSO(DevicePipelineStateObjectDX12&);
		void BindComputePSO(DevicePipelineStateObjectDX12&);
		void BindRTPSO(DeviceRTPipelineStateObjectDX12&);
		void BindGPUVisibleHeaps();
		void PrepareGPUVisibleHeaps(RenderPass& pass);
		void CommitStagedDescriptors();
		void GenerateMipmaps(Texture2D* tex);

		void BeginDrawFrameGraph(FrameGraph* fg) override;
		void EndDrawFrameGraph() override;
		void DrawRenderPass(RenderPass& pass) override;
		void PopulateCmdsFrom(FrameGraph* fg) override;

		void SetDynamicConstantBuffer(ConstantBufferBase* cb);

		void TransitionBarrier(shared_ptr<DeviceResourceDX12> resource, D3D12_RESOURCE_STATES stateAfter);

		CommandListComPtr GetDeviceCmdListPtr() { return m_CmdList; }
		DeviceDX12& GetDeviceDX12();

	private:
		void ExecuteMipmapRenderPasses();
		void PrepareGPUVisibleHeapForMipmap(RenderPass& pass, u32 targetMipLevel);
		CommandListDX12(Device& d, QueueType t);
		CommandAllocatorComPtr				m_CmdListAlloc;
		CommandListComPtr						m_CmdList;

		DynamicDescriptorHeapDX12			m_DynamicDescriptorHeaps[2] =
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		};

		friend class CommandQueueDX12;
		friend class DeviceDX12;

		Vector<std::pair<u32, RenderPass>> m_mipmapGenerationPass;
	};
}
