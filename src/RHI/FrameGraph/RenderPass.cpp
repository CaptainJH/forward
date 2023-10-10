//***************************************************************************************
// RenderPass.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "RenderPass.h"
#include "RenderPassHelper.h"
#include "Device.h"

using namespace forward;

RenderPass::RenderPass(RasterSetupFuncType setupFunc, ExecuteFuncType execute, OperationFlags operationType)
	: m_executeCallback(execute)
	, m_opFlags(operationType)
	, m_pso(RasterPipelineStateObject())
{
	RenderPassBuilder builder(this);
	setupFunc(builder, GetPSO<RasterPipelineStateObject>());
}
RenderPass::RenderPass(ComputeSetupFuncType setupFunc, ExecuteFuncType execute, OperationFlags operationType)
	: m_executeCallback(execute)
	, m_opFlags(operationType)
	, m_pso(ComputePipelineStateObject())
{
	RenderPassBuilder builder(this);
	setupFunc(builder, GetPSO<ComputePipelineStateObject>());
}
RenderPass::RenderPass(RTSetupFuncType setupFunc, ExecuteFuncType execute, OperationFlags operationType)
	: m_executeCallback(execute)
	, m_opFlags(operationType)
	, m_pso(RTPipelineStateObject())
{
	RenderPassBuilder builder(this);
	setupFunc(builder, GetPSO<RTPipelineStateObject>());
}
RenderPass::RenderPass(SceneData& sceneData, RTSetupFuncType setupFunc, ExecuteFuncType execute, OperationFlags operationType)
	: m_executeCallback(execute)
	, m_opFlags(operationType)
	, m_pso(RTPipelineStateObject(sceneData))
{
	RenderPassBuilder builder(this);
	setupFunc(builder, GetPSO<RTPipelineStateObject>());
}

RenderPass::OperationFlags RenderPass::GetRenderPassFlags() const
{
	return m_opFlags;
}

void RenderPass::Execute(CommandList& cmdList)
{
	m_executeCallback(cmdList);
}

void RenderPass::AttachRenderPass(RenderPass* ptr)
{
	m_nextPass = ptr;
}

RenderPass* RenderPass::GetNextRenderPass()
{
	return m_nextPass;
}
