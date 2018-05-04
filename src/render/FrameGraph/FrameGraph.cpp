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
}

std::vector<RenderPassInfo>& FrameGraph::GetRenderPassDB()
{
	return m_passDB;
}