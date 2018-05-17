//***************************************************************************************
// FrameGraph.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FrameGraph.h"

using namespace forward;

void FrameGraph::Reset()
{
	m_passDB.clear();
	m_allUsedResources.clear();
}

void FrameGraph::DrawRenderPass(RenderPass* pass)
{
	RenderPassInfo info;
	info.m_renderPass = pass;
	m_passDB.push_back(info);

	auto& pso = pass->GetPSO();
	info.m_vertexFormat = &pso.m_IAState.m_vertexLayout;

	for (auto& vb : pso.m_IAState.m_vertexBuffers)
	{
		if (vb)
		{
			//info.m_inputResources.push_back(vb.get());
		}
	}
}

std::vector<RenderPassInfo>& FrameGraph::GetRenderPassDB()
{
	return m_passDB;
}