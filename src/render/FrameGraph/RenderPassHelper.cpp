#include "RenderPassHelper.h"

using namespace forward;

RenderPassBuilder::RenderPassBuilder(RenderPass* renderPass)
	:m_renderPass(renderPass)
{}

RenderPassBuilder::~RenderPassBuilder()
{

}

RenderPassBuilder& RenderPassBuilder::operator<<(IRenderPassSource& src)
{
	src.OnRenderPassBuilding(*m_renderPass);
	return *this;
}

RenderPassBuilder& forward::operator<<(RenderPassBuilder& lhs, RenderPassBuilder& rhs)
{
	auto decorator = dynamic_cast<RenderPassBuilderDecorator*>(&rhs);
	assert(decorator);
	decorator->SetRenderPassBuilder(&lhs);
	return rhs;
}

RenderPass* RenderPassBuilder::GetRenderPass()
{
	return m_renderPass;
}

RenderPassBuilderDecorator::RenderPassBuilderDecorator()
	: RenderPassBuilder(nullptr)
{}

void RenderPassBuilderDecorator::SetRenderPassBuilder(RenderPassBuilder* builder)
{
	m_builder = builder;
}

RenderPass* RenderPassBuilderDecorator::GetRenderPass()
{
	return m_builder->GetRenderPass();
}

RenderPassBuilder& RenderPassBuilderDecorator::operator<<(IRenderPassSource& src)
{
	RenderPass dummy;
	src.OnRenderPassBuilding(dummy);
	PostDummyRenderPassBuilding(dummy);

	assert(m_builder);
	RenderPassBuilderDecorator* decorator = dynamic_cast<RenderPassBuilderDecorator*>(m_builder);
	if (decorator)
	{
		return decorator->operator<<(src);
	}
	else
	{
		return *m_builder;
	}
}

VertexBufferOnly forward::VBOnly;
void VertexBufferOnly::PostDummyRenderPassBuilding(RenderPass& dummy)
{
	GetRenderPass()->GetPSO().m_IAState.m_vertexBuffers = dummy.GetPSO().m_IAState.m_vertexBuffers;
}