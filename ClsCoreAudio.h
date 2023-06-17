#include "framework.h"
#include "ClsSilence.h"
/// <summary>
/// Rebuffer clas is used to keep the Data of the HardwareBuffer.
/// It can collect several Pakets of the HardwareBuffer.
/// Its needed because of we need a specific size per VideoFrame.
/// </summary>
struct ReBuffer
{
    vector<BYTE> m_pBuffer;
    size_t m_uiBufferSize;
    ReBuffer()
    {
        m_uiBufferSize = 0;
        m_pBuffer.clear();
    }
    /// <summary>
    /// Fills the Array with a Paket of the Audio HardwareBuffer
    /// </summary>
    /// <param name="pData">AudioData</param>
    /// <param name="uiBufferSize">Size of the AudioData</param>
    /// <returns></returns>
    size_t PushX(BYTE* pData, size_t uiBufferSize)
    {
        m_pBuffer.resize(m_uiBufferSize + uiBufferSize);

        if (pData)
        {
            memcpy(m_pBuffer.data() + m_uiBufferSize, pData, uiBufferSize);
            m_uiBufferSize = m_uiBufferSize + uiBufferSize;
        }
        
        return m_uiBufferSize;
    }//END-FUNC
    /// <summary>
    /// Returns the Number of keeped Frames
    /// </summary>
    /// <param name="uiAFrameSize">Size of one Frame</param>
    /// <returns>Number of keeped AudioFrames</returns>
    UINT CurrentAudioFrames(UINT uiAFrameSize)
    {
        return m_uiBufferSize / uiAFrameSize;
    }//END-FUNC
    /// <summary>
    /// Fetch a specific Datasize (specific Number of AudioFrames)
    /// and delete it from our RebufferArray.
    /// </summary>
    /// <param name="pTargetData">Pointer to the Data that we want to fetch</param>
    /// <param name="uiTargetSize">Size of the Data that we want we to fetch</param>
    /// <param name="bIsValid">checks if the Data is valid</param>
    /// <returns></returns>
    size_t PopX(BYTE* pTargetData, size_t uiTargetSize, BOOL bIsValid = false)
    {
        if (uiTargetSize > m_pBuffer.size())
            bIsValid = false;
        else
            bIsValid = true;
        if (uiTargetSize == 0)
            bIsValid = true;
        memcpy(pTargetData, m_pBuffer.data(), uiTargetSize);
        m_pBuffer.erase(m_pBuffer.begin(), m_pBuffer.begin() + uiTargetSize);
        m_uiBufferSize = m_uiBufferSize - uiTargetSize;
        return m_uiBufferSize;
    }//END-FUNC
};
class ClsCoreAudio
{
public:
    friend class ClsSinkWriter;
private:
    HANDLE m_hEventPlayingSilence;
    HANDLE m_hThreadPlaySilence;
    static UINT32 m_uiNumFrames;
    UINT32 m_uiNumFramesHWBuffer;
    static UINT32 m_uiPacketLength;
    static REFERENCE_TIME m_lCurDurationHardwareBuffer;
    REFERENCE_TIME m_lDurationHardwareBuffer;
    Events m_myEvents;
    static BYTE* m_pData;
    
    ComPtr <IMMDevice> m_pDevice;
    ComPtr <IMMDeviceEnumerator> m_pEnumerator;
    ComPtr <IAudioClient> m_pAudioClient;
    static ComPtr <IAudioCaptureClient> m_pCaptureClient;
    static WAVEFORMATEX* m_pWaveFormat;
public:
    ClsCoreAudio();
    ~ClsCoreAudio();
private:
    HRESULT InitEngine();
    void CalcCurrentDurationOfBuffer();
    static ReBuffer m_myRebuffer;
    static REFERENCE_TIME Ns100UnitsInMs(REFERENCE_TIME lValue);
    static HRESULT ReleaseBuffer();
    static DWORD WINAPI StartSilence(LPVOID pParm);
    HRESULT InitAudioClient(WAVEFORMATEX** ppWaveFormat);
    HRESULT PlaySilence();
    static HRESULT ReadBuffer(BYTE** pAudioData, UINT* pBufferSize, UINT* pFPS);
    HRESULT FinishStream();
    
};