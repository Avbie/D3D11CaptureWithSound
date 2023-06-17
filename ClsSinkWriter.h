#pragma once
#include "framework.h"
#include "ClsCoreAudio.h"

template <class T>
void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

struct DataForAudioThread
{
    BYTE** pAudioData;
    UINT* uiAudioBytes;
    UINT* uiFPS;
    HRESULT(**pReadAudioBuffer)(BYTE**, UINT*, UINT* uiFPS);
    HANDLE hEventReadAudioHWBuffer;

};

class ClsSinkWriter
{
private:
    // Video
    DWORD m_dwStreamIndexVidOut;                     // VideoSpur
    LONGLONG m_lDurationVid;                         // In 100NanoSecondsUnits how long one Frame will be displayed. Depending on FrameRate
    LONGLONG m_lSampleTimeVid;                       // Sum of m_lDurationVid, identify the Position of each Frame
    // Audio
    DWORD m_dwStreamIndexAudOut;                     // AudioSpur ( e.g. German, English etc.)
    LONGLONG m_lDurationAud;                         // In 100NanoSecondsUnits how long a AudioFrames needs for Playback
    LONGLONG m_lSampleTimeAud;                       // Sum of m_lDurationVid, identify the Position of each Frame

    ComPtr <IMFSinkWriter> m_pSinkWriter;
    
    FrameData* m_pFrameData;
    BOOL m_bFinished;
    
  
    // Video
    UINT32 m_uiFPS;                                         // FPS Limit
    UINT32 m_uiBitRate;                                     // Height*Width*Bpp
    DWORD m_dwDataRow;                                      // Width*Bpp
    PicDataBitReading m_myBitReading = PicDataBitReading::Standard; //Type of PixelData Interpretation (Bmp: Last to First Line)
    WCHAR m_wstrFilename[MAXSIZE] = L"";                    // VideoFileName as wChar
    GUID m_MyOutputFormat = MFVideoFormat_H264;             // OutputFormat
    GUID m_MyInputFormat = MFVideoFormat_RGB32;             // InputFormat
    ComPtr <IMFMediaBuffer> m_pBuffer;                      // IMF-Buffer für das Sample, nimmt pData von FrameData auf
    ComPtr <IMFSample> m_pSample;                           // IMF-Sample, builded with m_pBuffer Data

    // Audio
    BOOL m_bIsAudio;
    ClsCoreAudio CoreAudio;
    WAVEFORMATEX* m_pWaveFormat;
    //FunctionPointer for ClsCoreAudio::ReadBuffer(...)
    HRESULT(*m_pReadAudioBuffer)(BYTE**, UINT*, UINT* uiFPS);
    HANDLE m_hThreadReadAudioHWBuffer;
    BYTE* m_pAudioData;
    UINT32 m_uiAudioBytes;

   
public:
    DataForAudioThread m_myDataForAudioThread;
public:
    ClsSinkWriter();
    ~ClsSinkWriter();
    void SetFrameData(FrameData** pFrameData);              // FrameFormat Information Structure
    void SetBitReading(PicDataBitReading myBitReading);     // Sets Type of PixelData Interpretation (Bmp: Last to First Line)
    void SetFileName(const char* strFileName);
    void SetAudio(BOOL bAudio);
    HRESULT PrepareInputOutput();
    HRESULT StartRecording();
    HRESULT LoopRecording();
    HRESULT StopRecording();
    //Video
private:
    HRESULT WriteVideoDataSample(unsigned char* pFrameBuffer);
    void FlipFormat(DWORD& dwDataRow, unsigned char* pFrameBuffer, BYTE** pPosition);
    void CalcDurationVid();
public:
    void SetFPS(UINT32 uiFPS);
    void SetBitRate(UINT32 uiBitRate);
    void SetFormats(GUID MyInputFormat, GUID MyOutputFormat);
    LONGLONG GetVideoFrameDuration();
    // Audio
public:
    HRESULT StartReadAudioHWBufferThread();
private:
    WAVEFORMATEX** WaveFormat() { return &m_pWaveFormat; }
    void SetReadAudioHWBufferCallback(HRESULT(*pReadAudioBuffer)(BYTE**, UINT*, UINT*));
    void WaitForReadAudioHWBuffer();
    HRESULT WriteAudioDataSample();
    static DWORD WINAPI AudioHardwareBufferThread(LPVOID pParm);
};
