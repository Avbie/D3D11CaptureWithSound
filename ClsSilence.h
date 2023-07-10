#pragma once
#include "Framework.h"
//const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using Microsoft::WRL::ComPtr;

/// <summary>
/// Events will be transmitted per VoidPointer in ClsCoreAudio::PlaySilence() per CreateThread
/// Will be used to synchronize for playing silence and real sound
/// </summary>
struct Events
{
public:
    HANDLE hEventPlayingSilence;
    bool bStopped;
public:
    Events()
    {
        hEventPlayingSilence = NULL;
        bStopped = false;
    }
};

class ClsSilence
{
public:
    friend class ClsCoreAudio; 
private:
    void DeviceCollection(ComPtr<IMMDeviceEnumerator> pEnumerator);
    HRESULT BlankAudioPlayback(const void* lParm);
    HRESULT LoadAudioBuffer(const UINT32 bufferFrameCount, const BYTE* pData, const WAVEFORMATEX* pwfx, DWORD* dwFlags);
};