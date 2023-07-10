#pragma once
#include "framework.h"

using namespace std;
using namespace std::chrono;

/// <summary>
/// Class for sync. the Frames.
/// Here we set the Time for each loopPassage in the Mainloop.
/// It have to be the same time for every loopPassage in the Mainloop for having a constantly FrameRate.
/// </summary>
class ClsFPSSync
{
private:
    LONGLONG m_lDuration;
    time_point<steady_clock> m_myNextFrame;
public:
    ClsFPSSync();
public:
    void SetFrameDuaration(const LONGLONG lDurInMilliSeconds);
    void SleepUntilNextFrame();
    void Start();
    void operator ++();
private:
    void SetTimeForNextFrame();
};