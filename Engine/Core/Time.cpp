#include "Time.h"

namespace Bamboo {

	Time::Clock::time_point Time::s_startTime;
	Time::Clock::time_point Time::s_lastFrameTime;

	float Time::s_deltaTime = 0.0f;
	int Time::s_fps = 0;

	int Time::s_frameCount = 0;
	float Time::s_fpsTimer = 0.0f;
	
	void Time::Initialize() {
		s_startTime = Clock::now();
		s_lastFrameTime = s_startTime;
	
		s_deltaTime = 0.0f;
		s_frameCount = 0;
		s_fps = 0;
		s_fpsTimer = 0.0f;

	}

	void Time::Update() {
		auto currentTime = Clock::now();
		s_deltaTime = std::chrono::duration<float>(currentTime - s_lastFrameTime).count();
		s_lastFrameTime = currentTime;

		s_fpsTimer += s_deltaTime;

		s_frameCount++;
		if (s_fpsTimer >= 1.0f) {
			s_fps = s_frameCount;
			s_frameCount = 0;
			s_fpsTimer = 0.0f;
		}
	}


	float Time::GetDeltaTime() {
		return s_deltaTime;
	}

	float Time::GetTotalTime() {
		return std::chrono::duration<float>(s_lastFrameTime - s_startTime).count();
	}
	
	int Time::GetFps() {
		return s_fps;
	}


};