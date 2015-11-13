//--------------------------------------------------------------------------------
// Timer
//
//--------------------------------------------------------------------------------
#pragma once

#include <chrono>
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
		long long Runtime();  // in second
		float Elapsed(); // in ms
		int Framerate();
		int MaxFramerate();
		int FrameCount();
		float Frametime();	// in second

	private:
		typedef std::chrono::high_resolution_clock ClockType;
		int m_iFramesPerSecond;
		int m_iMaxFramesPerSecond;
		int m_iFrameCount;
		int m_iOneSeconedFrameCount;
		
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