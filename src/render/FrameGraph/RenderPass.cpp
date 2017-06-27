//***************************************************************************************
// RenderPass.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "RenderPass.h"

using namespace forward;

PipelineStateObject& RenderPass::GetPSO()
{
	return m_pso;
}

std::vector<FrameGraphResource>& RenderPass::GetFrameGraphResources()
{
	return m_resources;
}