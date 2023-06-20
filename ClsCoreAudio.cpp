#include "ClsCoreAudio.h"
/**********static Definitions*********/
UINT32 ClsCoreAudio::m_uiNumFrames = 0;
UINT32 ClsCoreAudio::m_uiPacketLength = 0;
REFERENCE_TIME ClsCoreAudio::m_lCurDurationHardwareBuffer = 0;
BYTE* ClsCoreAudio::m_pData = NULL;
ComPtr<IAudioCaptureClient> ClsCoreAudio::m_pCaptureClient = 0;
WAVEFORMATEX* ClsCoreAudio::m_pWaveFormat = NULL;
ReBuffer ClsCoreAudio::m_myRebuffer;
/*************************************/
/// <summary>
/// Constructor
/// </summary>
/// <param name="pCopyDataSample">FunctionPtr of SinkWriter WriteFrame</param>
ClsCoreAudio::ClsCoreAudio()
{
    // static
    m_uiNumFrames = 0;
    m_uiPacketLength = 0;
    m_lCurDurationHardwareBuffer = 0;
    m_pWaveFormat = NULL;
    m_pData = NULL;
    m_pCaptureClient.ReleaseAndGetAddressOf();

    // general
    m_hEventPlayingSilence = NULL;
    m_hThreadPlaySilence = NULL;
    m_uiNumFramesHWBuffer = 0;
    //  in 100er Nanosecond Units 300000 100erNsUnits = 30000MicroSeconds = 30Milliseconds
    m_lDurationHardwareBuffer = DEFAUDIOHWBUFFERSIZE;
    m_myEvents.bStopped = false;
    m_myEvents.hEventPlayingSilence = NULL;
   
    m_pDevice.ReleaseAndGetAddressOf();
    m_pEnumerator.ReleaseAndGetAddressOf();
    m_pAudioClient.ReleaseAndGetAddressOf();
}
/// <summary>
/// Destructor
/// </summary>
ClsCoreAudio::~ClsCoreAudio()
{
    CoTaskMemFree(m_pWaveFormat);
    m_pCaptureClient.Reset();
    m_pDevice.Reset();
    m_pEnumerator.Reset();
    m_pAudioClient.Reset();
}//END-CONS
/// <summary>
/// Calculate the Size of the Hardware AudioBuffer of the Soundcard
/// CalledBy: InitAudioClient(...)
/// </summary>
void ClsCoreAudio::CalcCurrentDurationOfBuffer()
{
    // Ermittelt die Buffersize als Zeitangabe in 100erNanosekundenEinheiten
    // Ist  Größe des akt. HardwareBuffers
    m_lCurDurationHardwareBuffer = (double)MULTIPL100NS * m_uiNumFramesHWBuffer / m_pWaveFormat->nSamplesPerSec;
}//END-FUNC
/// <summary>
/// Init. the AudioClient in Loopback-Mode. 
/// That means The AudioCaptureClient can get the Data as a HardwareBuffer by the AudioClient.
/// AudioClient provides the SoundFormat (its captured by Hardware -> no Filters, highest possible Format)
/// </summary>
/// <param name="ppWaveFormat">Pointer to Pointer init. in this Function</param>
/// <returns>HRESULT</returns>
HRESULT ClsCoreAudio::InitAudioClient(WAVEFORMATEX** ppWaveFormat)
{
    InitEngine();
    HRESULT hr = NULL;
    HR_RETURN_ON_ERR(hr, m_pEnumerator->GetDefaultAudioEndpoint(
        eRender,            // Datenrichtung, eRender = Ausgabe;  eCapture = Aufnahme (Mic)
        eConsole,           // Typ der Soundquelle eConsole = Games, SystemSounds, VoiceCmds; enum ERole
        &m_pDevice));        // Angabe des IF was per Polym. ein entsprechendes Obj erstellt

    HR_RETURN_ON_ERR(hr, m_pDevice->Activate(
        IID_IAudioClient,   // je nachdem welches IF ich mitgebe um ein Obj zu erstellen muss ich eine bestimmte ID angeben
        CLSCTX_ALL,         // Angabe in welchen Prozess/Thread das erstellte COM obj laufen soll
        NULL,               /* NULL bei (neu):  IAudioClient,
                            IAudioEndpointVolume: Lautstärke
                            IAudioMeterInformation: nPeakLevel, Audiospitzen überwachen bzw. max. festlegen (Hardware muss es unterstützen)
                            IAudioSessionManager: minimale funktionalität auf einen Audiostream ohne diesen direkt zu öffnen/eerstellen
                                                    mit RegisterAudioSessionNotification(pAudioEvents) erstelle ich ein Obj womit ich auf Volume und die Icons zugriff habe (Windows Volume anzeige)
                            IDeviceTopology: Zugriff auf den Pfad bis zum EndpointDevice (hier kann man evtl Sounds einzelner Anwendung abfangen bevor sie gemixt werden)
                            // https://learn.microsoft.com/en-us/windows/win32/coreaudio/device-topologies
                            // eine PROVARIANT Structure bei(alt): IBaseFilter, IDirectSound, IDirectSound8, IDirectSoundCapture, or IDirectSoundCapture8
                            // PROVARIANT Structure schaue aecSDKDemo project*/
        (void**)&m_pAudioClient)); // IF-Pointer der aufs erstellte Obj zeigt

    HR_RETURN_ON_ERR(hr, m_pAudioClient->GetMixFormat(&m_pWaveFormat)); // ruft die Eigenschaften der soundkarte ab, die intern auf low lvl ebene genutzt werden (samplerate, channels etc)
    *ppWaveFormat = m_pWaveFormat;

    // Set Buffersize in TimeUnits
    HR_RETURN_ON_ERR(hr, m_pAudioClient->Initialize(        // stream initialisieren, allocate Buffer
        AUDCLNT_SHAREMODE_SHARED,           // exclusive: ohne system sounds, shared: mit system sounds
        AUDCLNT_STREAMFLAGS_LOOPBACK,       // loopback geht nur in shared Mode
        m_lDurationHardwareBuffer,          // Set the Buffersize in TimeUnits
        0,
        m_pWaveFormat,                      // format description
        NULL));                             // bindet den stream an die audio session, bei null, neue session


    // Get BufferSize back in FramesCount
    HR_RETURN_ON_ERR(hr, m_pAudioClient->GetBufferSize(&m_uiNumFramesHWBuffer)); // Get the max. Buffersize back as FramesCount and not in TimeUnits
    // IF for more functionality
    HR_RETURN_ON_ERR(hr, m_pAudioClient->GetService(
        IID_IAudioCaptureClient,
        (void**)&m_pCaptureClient));

    CalcCurrentDurationOfBuffer();          // Größe/Länge des HardwareBuffers der Soundkarte

    return hr;
}//END-FUNC
/// <summary>
/// Inits a IMMDeviceEnumerator per CoCreateInstance.
/// - Will be init by COM-Interface (Binar). Inherits the Interface with the specific functions.
/// Enumerator can list any EndPointAudioDevices
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsCoreAudio::InitEngine()
{
    HRESULT hr = NULL;
    // Creates an Enum of AudioEndpointDevices
    HR_RETURN_ON_ERR(hr, CoCreateInstance(
        CLSID_MMDeviceEnumerator, // ID gibt an welches IF verwendet bzw. welches Obj. erstellt wird
        NULL,
        CLSCTX_ALL,
        /*CLSCTX_INPROC_SERVER - läuft im selben Prozess
            CLSCTX_INPROC_HANDLER - läuft im selben prozess wie das aufrufende Programm
            CLSCTX_LOCAL_SERVER
            CLSCTX_REMOTE_SERVER*/
        IID_PPV_ARGS(&m_pEnumerator)));
    return hr;
}//END-FUNC
/// <summary>
/// Finish the record/stream of Audio
/// CalledBy: SuperClass D3D11Recording::StopRecording()
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsCoreAudio::FinishStream()
{
    HRESULT hr = NULL;
    HR_RETURN_ON_ERR(hr, m_pAudioClient->Stop());           // Stop recording.

    m_myEvents.bStopped = true;                             // forces a break in the slientplay loop/thread
    WaitForSingleObject(m_hThreadPlaySilence, INFINITE);    // wait until the SilentPlayThread finished (Thread join)

    CloseHandle(m_hThreadPlaySilence);                      // thread handle schliessen
    CloseHandle(m_hEventPlayingSilence);
    return hr;
}//END-FUNC
/// <summary>
/// Starts a new Thread for Playing Silence
/// Info: Why? When we will record a Video with Sound and sometimes its silence, 
/// the HardwareBuffer/Soundcard will stop working. That means the CPU will close the Pipeline to the Soundcard.
/// Thats why we force the Hardware to be active and the Hardwarebuffer will be filled with Silence Sound.
/// CalledBy: Superclass ClsD3D11Recording::PrepareRecording()
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsCoreAudio::PlaySilence()
{
    HRESULT hr = NULL;

    // Creates an Event that is not set
    m_hEventPlayingSilence = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_myEvents.hEventPlayingSilence = m_hEventPlayingSilence;
    m_myEvents.bStopped = false;
    // New Thread calls StartSilence with m_myEvents as Param
    m_hThreadPlaySilence = CreateThread(NULL, 0, ClsCoreAudio::StartSilence, &m_myEvents, 0, NULL);
    // If Thread cannot be created
    if (NULL == m_hThreadPlaySilence)
    {
        printf("CreateThread failed: last error is %u\n", GetLastError());
        CloseHandle(m_hEventPlayingSilence);
        return E_FAIL;
    }//END-IF
    // wait until the other Thread playing silence and set the Event
    WaitForSingleObject(m_hEventPlayingSilence, INFINITE);
    // Start stream
    HR_RETURN_ON_ERR(hr, m_pAudioClient->Start());

    return hr;
}//END-FUNC
/// <summary>
/// Static Function. Will be set in ClsSinkWriter::SetReadAudioBufferCallback(...) as a FunctionPointer.
/// - This Functionpointer ClsSinkWriter::m_pReadAudioBufferwill be zipped in a struct 
///   called ClsSinkWriter::m_myDataForAudioThread
/// - This Struct with the included FunctionPointer will be transmitted to a new Thread
///   - The Thread is created in ClsSinkWriter::StartAudioHardwareBufferThread()
/// - The new Thread calls the Function ClsSinkWriter::AudioHardwareBufferThread
///   - The new Thread has now the Data of m_myDataForAudioThread
///   - This Struct includes the Functionspointer to ClsCoreAudio::ReadBuffer
/// Info: 
/// - Why? This Readbuffer Function includes a sleep-Function.
/// AudioHardwareBuffer needs some time for listening/saving new Sounddata.
/// - During the sleep all would be blocked, also the VideoFrame Capture MainThread
/// - we would lose VideoFrames
/// - Preventing of losing Videoframes during the sleep of the AudioHardwareBuffer, we let run the
///   AudioHardwareBuffer in a seperate Thread
/// </summary>
/// <param name="pAudioData">Data of AudioFrames for one VideoFrame</param>
/// <param name="pBufferSize">The Size of AudioFrames for one VideoFrame</param>
/// <param name="pFPS">VideoFPS</param>
/// <returns>HRESULT</returns>
HRESULT ClsCoreAudio::ReadBuffer(BYTE** pAudioData, UINT* pBufferSize, UINT* pFPS)
{
    HRESULT hr = NULL;
    DWORD dwFlags = 0;
    UINT uiFramesInReBuffer = 0;
    UINT uiTotalAudioBytes = 0;
    // 1 AudioFrame includes 2 Samples. 2* SampleSize = 1 AudioFrame
    UINT uiAFrameSize = m_pWaveFormat->nChannels * m_pWaveFormat->wBitsPerSample / 8;
    // Number of AudioFrames for one VideoFrame
    double uiAudioFramesPerVidFrame = (double)m_pWaveFormat->nSamplesPerSec / (double)*pFPS;
    UINT64 uiBeginFrame = 0;
    UINT64 uiEndFrame = 0;

    uiFramesInReBuffer = m_myRebuffer.CurrentAudioFrames(uiAFrameSize);
    // Loop as long the Number of AudioFrames is too small for one VideoFrame
    while (uiAudioFramesPerVidFrame > uiFramesInReBuffer)
    {
        /*
        * packetLength ist ein Paket von vielen im Hardwarebuffer
        * wenn ein Paket mal Größe 0 haben sollte, dann ist app aufm aktuellen stand wie die hardware
        * -> app kurz schlafen, derweil füllt sich hardwarebuffer wieder
        * -> neue pakete können abgeholt werden, bis app wieder auf hardware aufholt
        * -> |----------------------------| Hardwarebuffer
        *    |----|-----|----|----|---|---|
        *     pck1  pck2 pck3
        * pAudioClient->GetCurrentPadding gibt an wieviele frames gelesen wurden
        * NumFrames-padding = pakete übrig die noch nicht gelesen wurden
        */
        HR_RETURN_ON_ERR(hr, m_pCaptureClient->GetNextPacketSize(&m_uiPacketLength));
        if (m_uiPacketLength == 0)
        {
            // Sleep for half the buffer duration.
            // to large values will affect the MainLoop
            Sleep(Ns100UnitsInMs(m_lCurDurationHardwareBuffer) / 2);
            hr = m_pCaptureClient->GetNextPacketSize(&m_uiPacketLength);
        }//END-IF
        HR_RETURN_ON_ERR(hr, m_pCaptureClient->GetBuffer(
            &m_pData,
            &m_uiNumFrames,
            &dwFlags, &uiBeginFrame, &uiEndFrame));
        // Used Bytes of the current used HardwareBuffer
        uiTotalAudioBytes = m_uiNumFrames * uiAFrameSize;
        // Push the Data of the Hardware buffer in our Rebuffer
        m_myRebuffer.PushX(m_pData, uiTotalAudioBytes);
        // keep the total Number of Frames in the Rebuffer
        uiFramesInReBuffer = uiFramesInReBuffer + m_uiNumFrames;
        // must be released befor we can call GetBuffer again
        m_pCaptureClient->ReleaseBuffer(m_uiNumFrames);
    }//END-WHILE Frames
    // allocate the Bytes of AudioFrames that are needed for one VideoFrame
    *pAudioData = (BYTE*)malloc(uiAudioFramesPerVidFrame * uiAFrameSize);
    // Gets the Data of Audio for exactly one VideoFrame
    m_myRebuffer.PopX(*pAudioData, uiAudioFramesPerVidFrame * uiAFrameSize);
    *pBufferSize = uiAudioFramesPerVidFrame * uiAFrameSize;
    HR_RETURN_ON_ERR(hr, ReleaseBuffer());
    return hr;
}//END-FUNC
/// <summary>
/// Releases the HardwareBuffer
/// Must be called befor we can get the next one.
/// CalledBy: ReadBuffer()
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsCoreAudio::ReleaseBuffer()
{
    HRESULT hr = NULL;
    HR_RETURN_ON_ERR(hr, m_pCaptureClient->ReleaseBuffer(m_uiNumFrames));

    return hr;
}//END-FUNC
/// <summary>
/// Static Function, called by a new Thread.
/// CalledBy: PlaySilence()
/// </summary>
/// <param name="pParm">pParm is a "zipped" struct with Events</param>
/// <returns>always 0</returns>
DWORD WINAPI ClsCoreAudio::StartSilence(LPVOID pParm)
{
    ClsSilence oClsSilence;
    oClsSilence.BlankAudioPlayback(pParm);
    return 0;
}//END-FUNC
/// <summary>
/// Convertes it from 100er NanosecondUnits in Milliseconds
/// CalledBy: ReadBuffer(...)
/// 10000 100erNs = 1000 Microseconds = 1 Millisecond  
/// </summary>
/// <param name="lValue">Value in 100er Nanosecond Units</param>
/// <returns></returns>
REFERENCE_TIME ClsCoreAudio::Ns100UnitsInMs(REFERENCE_TIME l100NanosecondUnits)
{
    return l100NanosecondUnits / 10000;
}//END-FUNC