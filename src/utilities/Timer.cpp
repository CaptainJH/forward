
#include "Timer.h"

using namespace forward;
using namespace std;

Timer::Timer()
	: m_iFramesPerSecond(0)
	, m_iMaxFramesPerSecond(0)
	, m_iFrameCount(0)
	, m_bUseFixedStep(false)
	, m_bPaused(false)
{

}

Timer::~Timer()
{

}

void Timer::Update()
{

}

void Timer::Reset()
{

}

float Timer::Runtime()
{
	return 0.0f;
}

float Timer::Elapsed()
{
	return 0.0f;
}

int Timer::Framerate()
{
	return 0;
}

int Timer::MaxFramerate()
{
	return m_iMaxFramesPerSecond;
}

int Timer::FrameCount()
{
	return m_iFrameCount;
}

float Timer::Frametime()
{
	return 0.0f;
}

void Timer::PauseTimer(bool pause)
{
	m_bPaused = pause;
}

bool Timer::IsTimerPaused() const
{
	return m_bPaused;
}

void Timer::SetFixedTimeStep(float step)
{

}