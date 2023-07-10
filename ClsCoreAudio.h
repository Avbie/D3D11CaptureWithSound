#include "framework.h"
#include "ClsSilence.h"
/// <summary>
/// Rebuffer clas is used to keep the Data of the HardwareBuffer.
/// It can collect several Pakets of the HardwareBuffer.
/// Its needed because of we need a specific size per VideoFrame.
/// </summary>

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using Microsoft::WRL::ComPtr;
using namespace std;

struct ReBuffer
{
    size_t m_uiBufferSize;
    vector<BYTE> m_pBuffer;
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
    UINT64 CurrentAudioFrames(UINT uiAFrameSize)
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
    static UINT32 m_uiNumFrames;
    static UINT32 m_uiPacketLength;
    static double m_dCurDurationHardwareBuffer;
    static ReBuffer m_myRebuffer;
    static BYTE* m_pData;
    static WAVEFORMATEX* m_pWaveFormat;
    static ComPtr <IAudioCaptureClient> m_pCaptureClient;

    HANDLE m_hEventPlayingSilence;
    HANDLE m_hThreadPlaySilence;
    UINT32 m_uiNumFramesHWBuffer;
    REFERENCE_TIME m_lDurationHardwareBuffer;
    Events m_myEvents;    
    ComPtr <IMMDevice> m_pDevice;
    ComPtr <IMMDeviceEnumerator> m_pEnumerator;
    ComPtr <IAudioClient> m_pAudioClient;
    
public:
    ClsCoreAudio();
    ~ClsCoreAudio();
private:
    void CalcCurrentDurationOfBuffer();
    HRESULT InitAudioClient(WAVEFORMATEX** ppWaveFormat);
    HRESULT InitEngine();
    HRESULT FinishStream();
    HRESULT PlaySilence();
    
    static HRESULT ReadBuffer(BYTE** pAudioData, UINT* pBufferSize, UINT* pFPS);
    static HRESULT ReleaseBuffer();
    static DWORD WINAPI StartSilence(LPVOID pParm);
    static DWORD Ns100UnitsInMs(const double lValue); 
};