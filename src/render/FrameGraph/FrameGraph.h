//***************************************************************************************
// FrameGraph.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <vector>
#include "RenderPass.h"
#include "RenderPassHelper.h"

namespace forward
{
	class FrameGraph
	{
	public:

	private:
		std::vector<RenderPass> m_passDB;
	};
}