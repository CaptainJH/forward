#include "Effect.h"

using namespace forward;

Effect::Effect()
{
}

Effect::~Effect()
{}

Effect& Effect::SetPass(u8 passIndex)
{
	m_currentPassIndex = passIndex;
	return *this;
}

void Effect::DrawEffect(FrameGraph* fg)
{
	std::for_each(m_renderPassVec.begin(), m_renderPassVec.end(), [=](RenderPass& rp) {
		fg->DrawRenderPass(&rp);
		});
}