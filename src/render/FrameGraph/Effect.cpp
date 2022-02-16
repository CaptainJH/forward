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