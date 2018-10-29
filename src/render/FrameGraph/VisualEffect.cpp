#include "VisualEffect.h"

using namespace forward;

VisualEffect::VisualEffect()
{
}

VisualEffect::~VisualEffect()
{}

VisualEffect& VisualEffect::SetPass(u8 passIndex)
{
	m_currentPassIndex = passIndex;
	return *this;
}