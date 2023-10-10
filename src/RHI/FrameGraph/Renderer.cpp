#include "Renderer.h"

using namespace forward;

RendererBase& RendererBase::SetPass(u8 passIndex)
{
	m_currentPassIndex = passIndex;
	return *this;
}

void RendererBase::DrawEffect(FrameGraph* fg)
{
	std::ranges::for_each(m_renderPassVec, [=](RenderPass& rp) {
		fg->DrawRenderPass(&rp);
		});
}