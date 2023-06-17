#pragma once
#include "framework.h"

using namespace std;
using namespace std::chrono;

class ClsFPSSync
{
private:
    time_point<steady_clock> m_myNextFrame;
    LONGLONG m_lDuration;
private:
    void SetTimeForNextFrame()
    {
        m_myNextFrame = m_myNextFrame + std::chrono::milliseconds(m_lDuration);
    }
public:
    void SetFrameDuaration(LONGLONG lDurInMilliSeconds)
    {
        m_lDuration = lDurInMilliSeconds;
    }
    void Start()
    {
        m_myNextFrame = steady_clock::now();
    }

    void SleepUntilNextFrame()
    {
        std::this_thread::sleep_until(m_myNextFrame);
    }

    void operator ++()
    {
        SetTimeForNextFrame();
    }
};