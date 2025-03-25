
#include "Timer.h"

using namespace forward;
using namespace std;

Timer::Timer()
	: m_iFramesPerSecond(1)
	, m_iMaxFramesPerSecond(0)
	, m_iFrameCount(0)
	, m_iOneSeconedFrameCount(0)
	, m_bUseFixedStep(false)
	, m_deltaTime(chrono::milliseconds::zero())
{
	m_startTimePoint = m_clock.now();
	m_lastTimePoint = m_startTimePoint;
	m_currentTimePoint = m_startTimePoint;
	m_oneSecondTimePoint = m_startTimePoint;
}

Timer::~Timer()
{

}

void Timer::Tick()
{
	m_currentTimePoint = m_clock.now();
	m_deltaTime = m_currentTimePoint - m_lastTimePoint;
 	++m_iFrameCount;

	auto duration = 
		chrono::duration_cast<chrono::milliseconds>(m_currentTimePoint - m_oneSecondTimePoint).count();
	if (duration < 1000)
	{
		++m_iOneSeconedFrameCount;
	}
	else
	{
		m_iFramesPerSecond = m_iOneSeconedFrameCount;
		if (m_iFramesPerSecond > m_iMaxFramesPerSecond)
			m_iMaxFramesPerSecond = m_iFramesPerSecond;

		m_iOneSeconedFrameCount = 0;
		m_oneSecondTimePoint = m_currentTimePoint;
	}

	m_lastTimePoint = m_currentTimePoint;
}

void Timer::Reset()
{
	m_iFramesPerSecond = 1;
	m_iMaxFramesPerSecond = 0;
	m_iFrameCount = 0;
	m_iOneSeconedFrameCount = 0;
	m_deltaTime = chrono::milliseconds::zero();

	m_startTimePoint = m_clock.now();
	m_lastTimePoint = m_startTimePoint;
	m_currentTimePoint = m_startTimePoint;
	m_oneSecondTimePoint = m_startTimePoint;
}

f32 Timer::Runtime()
{
	auto length = m_currentTimePoint - m_startTimePoint;
	return chrono::duration_cast<chrono::milliseconds>(length).count() / 1000.0f;
}

f32 Timer::Elapsed()
{
	auto frameTime = chrono::duration_cast<chrono::microseconds>(m_deltaTime).count();
	return frameTime / 1000.0f;
}

i32 Timer::Framerate()
{
	return m_iFramesPerSecond;
}

i32 Timer::MaxFramerate()
{
	return m_iMaxFramesPerSecond;
}

i32 Timer::FrameCount()
{
	return m_iFrameCount;
}

f32 Timer::Frametime()
{
	return (1.0f / m_iFramesPerSecond);
}

