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

void RenderPass::Reset()
{
	m_resources.clear();

	m_pso.m_blendState = BlendState();
	m_pso.m_depthStencilState = DepthStencilState();
	m_pso.m_rasterizerState = RasterizerState();
}

void RenderPass::AddFrameGraphResource(const std::string& name)
{
	FrameGraphResource resource(name);

	m_resources.push_back(resource);
}