#include <core/timerClass.h>

void timerClass::updateCheckpoint()
{
	m_checkpoint = ChronoHRC::now();
}

float timerClass::getTimeSinceCheckpoint()
{
	ChronoHRC::time_point current = ChronoHRC::now();
	
	return std::chrono::duration<float>(current - m_checkpoint).count();
}