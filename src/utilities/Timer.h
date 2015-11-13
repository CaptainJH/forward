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

		void Update();
		void Reset();
		float Runtime();
		float Elapsed();
		int Framerate();
		int MaxFramerate();
		int FrameCount();
		float Frametime();
		void PauseTimer(bool pause);
		bool IsTimerPaused() const;

		void SetFixedTimeStep( float step );

	private:
		typedef std::chrono::high_resolution_clock ClockType;
		int m_iFramesPerSecond;
		int m_iMaxFramesPerSecond;
		int m_iFrameCount;
		
		ClockType::duration m_fixedDuration;
		bool m_bUseFixedStep;
		bool m_bPaused;

		ClockType::duration m_deltaTime;
		std::chrono::time_point<ClockType> m_startTimePoint;
		std::chrono::time_point<ClockType> m_lastTimePoint;
		std::chrono::time_point<ClockType> m_currentTimePoint;
	};
};
//--------------------------------------------------------------------------------