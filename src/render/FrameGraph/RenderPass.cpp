//***************************************************************************************
// RenderPass.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "RenderPass.h"
#include "render.h"

using namespace forward;

PipelineStateObject& RenderPass::GetPSO()
{
	return m_pso;
}

RenderPass::RenderPass(CleanType cleanType, SetupFuncType setup, ExecuteFuncType execute)
	: m_cleanType(cleanType)
	, m_setupCallback(setup)
	, m_executeCallback(execute)
{
	m_setupCallback(GetPSO());
}

RenderPass::~RenderPass()
{

}

RenderPass::CleanType RenderPass::GetCleanType() const
{
	return m_cleanType;
}

void RenderPass::Execute(Renderer& render)
{
	m_executeCallback(render);
}