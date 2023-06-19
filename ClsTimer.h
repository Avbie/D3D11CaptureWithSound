#pragma once
#include <chrono>

/// <summary>
/// Used in ClsD3D11::SetConstantBuffer()
/// - Testscenario: image rotation per Frame depending on Time.
/// - Constantbuffer will just upload a Function/Matrix every Frame
///   to the GPU instead the Frame itself.
/// </summary>
class ClsTimer
{
public:
	ClsTimer() noexcept;
	float Mark() noexcept;
	float Peek() const noexcept;
private:
	std::chrono::steady_clock::time_point m_myTimeStamp;
};