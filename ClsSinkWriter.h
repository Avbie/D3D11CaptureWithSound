#pragma once
#include "framework.h"
#include "FrameData.h"
#include "ClsCoreAudio.h"

/// <summary>
/// used for communication between ClsSinkWriter and ClsCoreAudio
/// </summary>
struct DataForAudioThread
{
    BYTE** pAudioData;
    UINT* uiAudioBytes;
    UINT* uiFPS;
    HRESULT(**ppReadAudioBuffer)(BYTE**, UINT*, UINT* uiFPS);
    //HANDLE hEventReadAudioHWBuffer;
};

class ClsSinkWriter
{
private:
    // generall
    //BOOL m_bFinished;
    ComPtr <IMFSinkWriter> m_pSinkWriter;
    FrameData* m_pFrameData;
    // Video
    DWORD m_dwDataRow;                              // Width*Bpp
    DWORD m_dwStreamIndexVidOut;                    // VideoSpur
    UINT32 m_uiBitRate;                             // Height*Width*Bpp
    //UINT32 m_uiFPS;                                 // FPS Limit
    LONGLONG m_lDurationVid;                        // In 100NanoSecondsUnits how long one Frame will be displayed. Depending on FrameRate
    LONGLONG m_lSampleTimeVid;                      // Sum of m_lDurationVid, identify the Position of each Frame
    WCHAR m_wstrFilename[MAXSIZE] = L"";            // VideoFileName as wChar
    
    PicDataBitReading m_myBitReading;               //Type of PixelData Interpretation (Bmp: Last to First Line)
    GUID m_myInputFormat;                           // InputFormat
    GUID m_myOutputFormat;                          // OutputFormat
    ComPtr <IMFMediaBuffer> m_pBuffer;              // IMF-Buffer für das Sample, nimmt pData von FrameData auf
    ComPtr <IMFSample> m_pSample;                   // IMF-Sample, builded with m_pBuffer Data
    
    // Audio
    //BOOL m_bIsAudio;
    HANDLE m_hThreadReadAudioHWBuffer;
    DWORD m_dwStreamIndexAudOut;                    // AudioSpur ( e.g. German, English etc.)
    UINT32 m_uiAudioBytes;
    LONGLONG m_lDurationAud;                        // In 100NanoSecondsUnits how long a AudioFrames needs for Playback
    LONGLONG m_lSampleTimeAud;                      // Sum of m_lDurationVid, identify the Position of each Frame

    ClsCoreAudio m_myClsCoreAudio;
    WAVEFORMATEX* m_pWaveFormat;
    BYTE* m_pAudioData;
    //FunctionPointer for ClsCoreAudio::ReadBuffer(...)
    HRESULT(*m_pReadAudioBuffer)(BYTE**, UINT*, UINT* uiFPS);
    DataForAudioThread m_myDataForAudioThread;

public:
    ClsSinkWriter();
    ~ClsSinkWriter();
    
    /*********General*****************/
    HRESULT LoopRecording();
    HRESULT StartRecording();
    HRESULT StopRecording();
    HRESULT PrepareInputOutput();
    /*********Video*******************/
public: 
    void SetBitRate();
    void SetBitReading(PicDataBitReading myBitReading);     // Sets Type of PixelData Interpretation (Bmp: Last to First Line)
    void SetFileName(const char* strFileName);
    //void SetFPS(UINT32 uiFPS);
    void CalcDurationVid();
    void SetFormats(GUID MyInputFormat, GUID MyOutputFormat);
    void SetFrameData(FrameData** pFrameData);              // FrameFormat Information Structure
    LONGLONG GetVideoFrameDuration();
private:
    
    void FlipFormat(DWORD& dwDataRow, unsigned char* pFrameBuffer, BYTE** pPosition);
    HRESULT WriteVideoDataSample(unsigned char* pFrameBuffer);
    /*********Audio*******************/
public:
    void PrepareAudio();
    //void SetAudio(BOOL bAudio);
    HRESULT StartReadAudioHWBufferThread();
private:
    void SetReadAudioHWBufferCallback(HRESULT(*pReadAudioBuffer)(BYTE**, UINT*, UINT*));
    void WaitForReadAudioHWBuffer();
    HRESULT WriteAudioDataSample();
    static DWORD WINAPI AudioHardwareBufferThread(LPVOID pParm);
    ClsCoreAudio& CoreAudio();
    WAVEFORMATEX** WaveFormat();
   
    
};
