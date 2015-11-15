//--------------------------------------------------------------------------------
// Timer
//
//--------------------------------------------------------------------------------
#pragma once

#include <chrono>
#include "Types.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Timer
	{
	public:
		Timer();
		~Timer();

		void Tick();
		void Reset();
		i64 Runtime();  // in second
		f32 Elapsed(); // in ms
		i32 Framerate();
		i32 MaxFramerate();
		i32 FrameCount();
		f32 Frametime();	// in second

	private:
		typedef std::chrono::high_resolution_clock ClockType;
		i32 m_iFramesPerSecond;
		i32 m_iMaxFramesPerSecond;
		i32 m_iFrameCount;
		i32 m_iOneSeconedFrameCount;
		
		ClockType m_clock;
		ClockType::duration m_fixedDuration;
		bool m_bUseFixedStep;

		ClockType::duration m_deltaTime;
		std::chrono::time_point<ClockType> m_startTimePoint;
		std::chrono::time_point<ClockType> m_lastTimePoint;
		std::chrono::time_point<ClockType> m_currentTimePoint;
		std::chrono::time_point<ClockType> m_oneSecondTimePoint;
	};
};
//--------------------------------------------------------------------------------