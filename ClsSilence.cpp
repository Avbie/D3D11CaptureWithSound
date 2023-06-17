#include "ClsSilence.h"

/// <summary>
/// Generate a Silence sound or a "pieep" sound
/// CalledBy: BlankAudioPlayback(..)
/// </summary>
/// <param name="bufferFrameCount">Number of Frames</param>
/// <param name="pData">Buffer of the Frames</param>
/// <param name="pwfx">WaveFormat</param>
/// <param name="outFlags">returned Flags</param>
/// <returns>HRESULT</returns>
HRESULT ClsSilence::LoadAudioBuffer(UINT32 bufferFrameCount, BYTE* pData, WAVEFORMATEX* pWaveFormat, DWORD* dwFlags)
{
    double dFrequency = 1700;		// 1.7 kHz
    double dNumChannels = 2;
    double dPhase = 0.0;
    double dSampleRate = 48000;
    double dPhaseInc = 0;
    float* pOutput = NULL;

    if (pWaveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
    {
        // AudioEngine kann nicht geöffnet werden, spielt keine Silence hab
        *dwFlags = AUDCLNT_BUFFERFLAGS_SILENT;
        return E_FAIL;
    }

    WAVEFORMATEXTENSIBLE* pWaveFormatWide = (WAVEFORMATEXTENSIBLE*)pWaveFormat;
    if (pWaveFormatWide->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
    {
        // AudioEngine kann nicht geöffnet werden, spielt keine Silence hab
        *dwFlags = AUDCLNT_BUFFERFLAGS_SILENT;
        return E_FAIL;
    }
    // ok now we know we can fill it with float data!
    pOutput = (float*)pData;

    // Compute the phase increment for the current frequency, the piep
    dPhaseInc = 2 * M_PI * dFrequency / dSampleRate;

    // Generate the samples
    for (UINT32 i = 0; i < bufferFrameCount; i++)
    {
        // Each Frame consists of two Floatvalues (Stereo)
        // All Frames will have the same Value, the same "piep"
        float x = float(0.1 * sin(dPhase));
        for (int ch = 0; ch < dNumChannels; ch++) // Stereo
            *pOutput++ = x;     
        dPhase += dPhaseInc;
    }

    // Bring phase back into range [0, 2pi]
    dPhase = fmod(dPhase, 2 * M_PI);

    return S_OK;
}//END-FUNC

/// <summary>
/// Creates AudioEndpointDevice
/// Creates an AudioClient and AudioRenderClient for the Stream
/// Calls LoadAudioBuffer that generates a SilenceSound
/// EventControlled By ClsCoreAudio
/// Plays Silence when ClsCoreAudio recording streams
/// Called by: ClsCoreAudio::StartStream() ---> Called by created Thread
/// </summary>
/// <param name="lParm">EventHandle from the MainThread</param>
HRESULT ClsSilence::BlankAudioPlayback(void* lParm)
{
    BOOL bStartEvent = false;
    HRESULT hr = NULL;
    DWORD dwFlags = 0;
    UINT32 uiBufferFrameCount = 0;
    UINT32 uiNumFramesAvailable = 0;
    UINT32 uiNumFramesPadding = 0;
    REFERENCE_TIME lDuration = 0;
    REFERENCE_TIME lDurationHardwareBuffer = REFTIMES_PER_MS;	// 10.000 100erNsUnits = 1.000.000 ns = 1ms
    BYTE* pData;                                                // Pointer to an intern Pointer to the HardwareBuffer
    WAVEFORMATEX* pWaveFormat = NULL;
    ComPtr<IMMDeviceEnumerator> pEnumerator = NULL;
    ComPtr<IMMDevice> pDevice = NULL;
    ComPtr<IAudioClient> pAudioClient = NULL;
    ComPtr<IAudioRenderClient> pRenderClient = NULL;



    Events* pStruMyEvents;
    pStruMyEvents = (Events*)lParm;                             // Struct of the MainThread

    HR_RETURN_ON_ERR(hr,
        CoCreateInstance(
            CLSID_MMDeviceEnumerator,                           // Code-Iden. that will be used to create the Obj
            NULL,
            CLSCTX_ALL,                                         // Angabe in welchen prozess es läuft
            IID_PPV_ARGS(&pEnumerator)));
            //IID_IMMDeviceEnumerator,                    
            //(void**)&pEnumerator));

    HR_RETURN_ON_ERR(hr,
        pEnumerator->GetDefaultAudioEndpoint(
            eRender,                                            // Datenrichtung, eRender = Ausgabe;  eCapture = Aufnahme (Mic)
            eConsole,                                           // Typ der Soundquelle eConsole = Games, SystemSounds, VoiceCmds; enum ERole
            &pDevice));                                         // Angabe des IF was per Polym. ein entsprechendes Obj erstellt

    HR_RETURN_ON_ERR(hr,
        pDevice->Activate(                                      // Enables specific Interfaces for the Device
            IID_IAudioClient,                                   // IAudioClient IF
            CLSCTX_ALL,                                         // Process Properties
            NULL,                                               // NULL on IAudioClient and some other IF
            (void**)&pAudioClient));                            // IF-Pointer

    HR_RETURN_ON_ERR(hr, 
        pAudioClient->GetMixFormat(&pWaveFormat));              // Native Format of the Soundcard/Engine

    HR_RETURN_ON_ERR(hr,                                        // AudioStream init.
        pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,                           // Usermode (not System/KernelMode)
            0,
            lDurationHardwareBuffer,                            // Set the HardwareBuffersize in TimeUnits
            0,                                                  // Priority
            pWaveFormat,                                  // IF-Pointer
            NULL));                                             // NULL= new AudioSession, or an GUID-struct for an AudioSession
    
    // Get the actual size of the allocated Hardwarebuffer in Frames
    HR_RETURN_ON_ERR(hr, 
        pAudioClient->GetBufferSize(&uiBufferFrameCount));      // Get the BufferSize back in FrameUnits (initialize was set in TimeUnit)

    HR_RETURN_ON_ERR(hr, 
        pAudioClient->GetService(                               // Extra Interface for more options
            IID_IAudioRenderClient,
            (void**)&pRenderClient));
    
    // Get the Buffer of the Frames
    HR_RETURN_ON_ERR(hr, 
        pRenderClient->GetBuffer(                               // Get FrameData that are ready to be written
            uiBufferFrameCount, 
            &pData));

    // Set pData to silence or "pieeep"
    HR_RETURN_ON_ERR(hr, LoadAudioBuffer(                       // Write SilenceNoise into the FrameData
        uiBufferFrameCount, 
        pData, 
        pWaveFormat,
        &dwFlags));

    HR_RETURN_ON_ERR(hr, pRenderClient->ReleaseBuffer(          // Release the written Frames
        uiBufferFrameCount, 
        dwFlags));
   
    // Calculate the actual duration of the allocated buffer.   // its just a small part of a Second
    lDuration = (REFERENCE_TIME)((double)REFTIMES_PER_MS * uiBufferFrameCount / pWaveFormat->nSamplesPerSec);
    HR_RETURN_ON_ERR(hr, pAudioClient->Start());  // Start playing.
   
    // Each loop fills about half of the shared buffer.
    while (dwFlags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
        // Sleep for half the buffer duration.
        // Sleep Angabe in MS, lDuration hat als Einheit 100erNsUnits = 1ms für alle anderen Wnd-Funktionen
        // Für Sleep ist es direkt in ms:  REFTIMES_PER_MS=10000; 10000/10 = 1000 = 1ms; 1ms/2 = 0,5ms
        Sleep((DWORD)(lDuration /10/ 2));

        // gibt an wieviele frames gelesen wurden
        HR_RETURN_ON_ERR(hr, pAudioClient->GetCurrentPadding(&uiNumFramesPadding));
        // Frames die noch übrig sind bzw. noch nicht gelesen wurden
        uiNumFramesAvailable = uiBufferFrameCount - uiNumFramesPadding;
        // Grab all the available space/buffer in the shared buffer.
        HR_RETURN_ON_ERR(hr, pRenderClient->GetBuffer(uiNumFramesAvailable, &pData));
        // Write SilenceNoise into the FrameData
        HR_RETURN_ON_ERR(hr, LoadAudioBuffer(uiNumFramesAvailable, pData, pWaveFormat, &dwFlags));
        // Release the written Frames
        HR_RETURN_ON_ERR(hr, pRenderClient->ReleaseBuffer(uiNumFramesAvailable, dwFlags));

        // Silence is playing, tell it the Mainthread that he can progress
        if (!bStartEvent)
            bStartEvent = SetEvent(pStruMyEvents->hEventPlayingSilence);
        // Stops if the MainThread will stop
        if (pStruMyEvents->bStopped)
            break;
    }

    // Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(lDuration / 10 / 2));
    HR_RETURN_ON_ERR(hr, pAudioClient->Stop());
    CoTaskMemFree(pWaveFormat);
}//END-FUNC