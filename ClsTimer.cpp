#include "ClsTimer.h"

using namespace std::chrono;

ClsTimer::ClsTimer() noexcept
{
	m_MyTimeStamp = steady_clock::now();
}

float ClsTimer::Mark() noexcept
{
	const auto MyOldTimeStamp = m_MyTimeStamp;
	m_MyTimeStamp = steady_clock::now();
	const duration<float> frameTime = m_MyTimeStamp - MyOldTimeStamp;
	return frameTime.count();
}

float ClsTimer::Peek() const noexcept
{
	return duration<float>(steady_clock::now() - m_MyTimeStamp).count();
}