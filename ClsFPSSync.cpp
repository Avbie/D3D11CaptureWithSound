#include "ClsFPSSync.h"

/// <summary>
/// Constructor
/// Object will be build in Superclass ClsD3D11Recording::ClsD3D11Recording(...)
/// </summary>
ClsFPSSync::ClsFPSSync()
{
    m_lDuration = 0;
    m_myNextFrame = steady_clock::now();
}//END-CONS
/// <summary>
/// - Sets the Duration for the Frame
/// - e.g. 60FPS 1/60 = 16ms for one Frame in Mainloop m_lDuration = 16
/// CalledBy: ClsD3D11Recording::ClsD3D11Recording
/// </summary>
/// <param name="lDurInMilliSeconds"></param>
void ClsFPSSync::SetFrameDuaration(LONGLONG lDurInMilliSeconds)
{
    m_lDuration = lDurInMilliSeconds;
}//END-FUNC
/// <summary>
/// - Sets a Timestamp of the first MainLoopPassage
/// - called right befor we enter the Mainloop
/// CalledBy: ClsD3D11Recording::PrepareRecording()
/// </summary>
void ClsFPSSync::Start()
{
    m_myNextFrame = steady_clock::now();
}//END-FUNC

/// <summary>
/// - Wait until the time for 1 Frame is past
/// CalledBy: ClsD3D11Recording::Recording() in MainLoop
/// </summary>
void ClsFPSSync::SleepUntilNextFrame()
{
    std::this_thread::sleep_until(m_myNextFrame);
}//END-FUNC
/// <summary>
/// Calculates the ending of the Timestamp of the Frame
/// CalledBy: ClsD3D11Recording::Recording() in MainLoop
/// </summary>
void ClsFPSSync::operator ++()
{
    SetTimeForNextFrame();
}//END-FUNC

/// <summary>
/// Calculates the ending of the Timestamp of the Frame
/// CalledBy: ++Operator in ClsD3D11Recording::Recording() in MainLoop
/// </summary>
void ClsFPSSync::SetTimeForNextFrame()
{
    m_myNextFrame = m_myNextFrame + std::chrono::milliseconds(m_lDuration);
}//END-FUNC