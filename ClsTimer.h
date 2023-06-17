#pragma once
#include <chrono>

class ClsTimer
{
public:
	ClsTimer() noexcept;
	float Mark() noexcept;
	float Peek() const noexcept;
private:
	std::chrono::steady_clock::time_point m_MyTimeStamp;
};