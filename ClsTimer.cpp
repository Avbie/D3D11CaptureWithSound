#include "ClsTimer.h"

using namespace std::chrono;

/// <summary>
/// Constructor
/// - keeps the current time in nanoseconds
/// </summary>
ClsTimer::ClsTimer() noexcept
{
	m_myTimeStamp = steady_clock::now();
}

/// <summary>
/// Difference between the old and a new Timestamp in Seconds
/// Not used yet
/// </summary>
/// <returns>Difference in Seconds</returns>
float ClsTimer::Mark() noexcept
{
	const auto myOldTimeStamp = m_myTimeStamp;
	m_myTimeStamp = steady_clock::now();
	const duration<float> frameTime = m_myTimeStamp - myOldTimeStamp;
	return frameTime.count();
}//END-FUNC

/// <summary>
/// Same Function like Mark()
/// </summary>
/// <returns></returns>
float ClsTimer::Peek() const noexcept
{
	return duration<float>(steady_clock::now() - m_myTimeStamp).count();
}//END-FUNC