#pragma once
#include <chrono>

#define ChronoHRC std::chrono::high_resolution_clock

class timerClass
{
public:
	void updateCheckpoint();
	float getTimeSinceCheckpoint();

private:
	ChronoHRC::time_point m_checkpoint = ChronoHRC::now();
};