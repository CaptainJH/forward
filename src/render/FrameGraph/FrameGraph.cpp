//***************************************************************************************
// FrameGraph.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FrameGraph.h"

using namespace forward;

void FrameGraph::Reset()
{
	m_passDB.clear();
}

void FrameGraph::DrawRenderPass(RenderPass* pass)
{
	m_passDB.push_back(pass);
}

void FrameGraph::Compile()
{

}

std::vector<RenderPass*>& FrameGraph::GetRenderPassDB()
{
	return m_passDB;
}