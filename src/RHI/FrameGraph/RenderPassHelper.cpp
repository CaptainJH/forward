#include "RenderPassHelper.h"

using namespace forward;

RenderPassBuilder::RenderPassBuilder(RenderPass* renderPass)
	:m_renderPass(renderPass)
{}

RenderPassBuilder& RenderPassBuilder::operator<<(IRenderPassGenerator& src)
{
	src.OnRenderPassBuilding(*m_renderPass);
	return *this;
}

RenderPass* RenderPassBuilder::GetRenderPass()
{
	return m_renderPass;
}