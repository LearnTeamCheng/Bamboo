#pragma once
#include <chrono>
namespace Bamboo {
	class Time
	{
	public:
		static void Initialize();
		static void Update();
		static float GetDeltaTime();
		static float GetTotalTime();
		static int GetFps();
	private:
		using Clock = std::chrono::steady_clock;
		static Clock::time_point s_startTime;
		static Clock::time_point s_lastFrameTime;
		static float s_deltaTime;
		static int s_fps;
		static float s_fpsTimer;
		static int s_frameCount;

	};
}

