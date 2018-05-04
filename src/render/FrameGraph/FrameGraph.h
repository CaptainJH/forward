//***************************************************************************************
// FrameGraph.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <vector>
#include "RenderPass.h"
#include "RenderPassHelper.h"

namespace forward
{
	struct RenderPassInfo
	{
		RenderPass* m_renderPass = nullptr;
		bool m_enabled = true;
	};

	class FrameGraph
	{
	public:

		void Reset();
		void Compile();
		void DrawRenderPass(RenderPass* pass);
		std::vector<RenderPass*>& GetRenderPassDB();

	private:
		std::vector<RenderPass*> m_passDB;
	};
}