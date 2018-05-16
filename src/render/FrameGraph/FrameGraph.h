//***************************************************************************************
// FrameGraph.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <vector>
#include "RenderPass.h"
#include "RenderPassHelper.h"

namespace forward
{
	struct RenderPassInfo;

	struct FrameGraphResourceInfo
	{
		FrameGraphResource*	m_resource;

		std::vector<RenderPassInfo*> m_readerPassList;
		std::vector<RenderPassInfo*> m_writerPassList;
	};

	struct RenderPassInfo
	{
		RenderPass* m_renderPass = nullptr;

		std::vector<FrameGraphResourceInfo*> m_inputResources;
		std::vector<FrameGraphResourceInfo*> m_outputResources;
	};

	class FrameGraph
	{
	public:

		void Reset();
		void DrawRenderPass(RenderPass* pass);
		std::vector<RenderPassInfo>& GetRenderPassDB();

	private:
		std::vector<RenderPassInfo> m_passDB;

		std::vector<FrameGraphResourceInfo> m_allUsedResources;
	};
}