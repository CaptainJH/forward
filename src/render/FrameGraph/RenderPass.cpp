//***************************************************************************************
// RenderPass.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "RenderPass.h"
#include "RenderPassHelper.h"
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
	RenderPassBuilder builder(this);
	m_setupCallback(builder, GetPSO());
}

RenderPass::RenderPass()
	: m_cleanType(RenderPass::CT_Default)
	, m_setupCallback(nullptr)
	, m_executeCallback(nullptr)
{}

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

void RenderPass::AttachRenderPass(RenderPass* ptr)
{
	m_nextPass = ptr;
}

RenderPass* RenderPass::GetNextRenderPass()
{
	return m_nextPass;
}