#include "Renderer.h"

using namespace forward;

RendererBase::RendererBase()
{
}

RendererBase::~RendererBase()
{}

RendererBase& RendererBase::SetPass(u8 passIndex)
{
	m_currentPassIndex = passIndex;
	return *this;
}

void RendererBase::DrawEffect(FrameGraph* fg)
{
	std::for_each(m_renderPassVec.begin(), m_renderPassVec.end(), [=](RenderPass& rp) {
		fg->DrawRenderPass(&rp);
		});
}