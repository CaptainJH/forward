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

RenderPass::RenderPass(OperationFlags flags, SetupFuncType setup, ExecuteFuncType execute)
	: m_opFlags(flags)
	, m_setupCallback(setup)
	, m_executeCallback(execute)
{
	RenderPassBuilder builder(this);
	m_setupCallback(builder, GetPSO());
}

RenderPass::RenderPass(SetupFuncType setup, ExecuteFuncType execute)
	: m_opFlags(RenderPass::OF_DEFAULT)
	, m_setupCallback(setup)
	, m_executeCallback(execute)
{
	RenderPassBuilder builder(this);
	m_setupCallback(builder, GetPSO());
}

RenderPass::RenderPass()
	: m_opFlags(RenderPass::OF_DEFAULT)
	, m_setupCallback(nullptr)
	, m_executeCallback(nullptr)
{}

RenderPass::~RenderPass()
{

}

RenderPass::OperationFlags RenderPass::GetRenderPassFlags() const
{
	return m_opFlags;
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