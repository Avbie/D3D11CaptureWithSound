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
    HRESULT BlankAudioPlayback(void* lParm);
    HRESULT LoadAudioBuffer(UINT32 bufferFrameCount, BYTE* pData, WAVEFORMATEX* pwfx, DWORD* outFlags);
};