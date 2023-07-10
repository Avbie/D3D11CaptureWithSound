#include "ClsSinkWriter.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using Microsoft::WRL::ComPtr;

/// <summary>
/// Constructor
/// Inits Library for the SinkWriter
/// </summary>
ClsSinkWriter::ClsSinkWriter()
{
   // m_bFinished = false;
    m_pSinkWriter.ReleaseAndGetAddressOf();
    m_pFrameData = NULL;
    // Video
    m_dwDataRow = 0;                                    // Width*Bpp
    m_dwStreamIndexVidOut = 0;                          // VideoSpur
    //m_uiFPS = 0;                                        // FPS Limit
    m_uiBitRate = 0;                                    // Height*Width*Bpp
    m_lDurationVid = 0;                                 // In 100NanoSecondsUnits how long one Frame will be displayed. Depending on FrameRate
    m_lSampleTimeVid = 0;                               // Sum of m_lDurationVid, identify the Position of each Frame

    m_myBitReading = PicDataBitReading::Standard;       // Type of PixelData Interpretation (Bmp: Last to First Line)
    m_myInputFormat = MFVideoFormat_RGB32;              // InputFormat
    m_myOutputFormat = MFVideoFormat_H264;              // OutputFormat
    m_pBuffer.ReleaseAndGetAddressOf();                 // IMF-Buffer für das Sample, nimmt pData von FrameData auf
    m_pSample.ReleaseAndGetAddressOf();                 // IMF-Sample, builded with m_pBuffer Data

    // Audio
    //m_bIsAudio = false;                                     
    m_hThreadReadAudioHWBuffer = NULL;
    m_dwStreamIndexAudOut = 0;                          // AudioSpur ( e.g. German, English etc.)
    m_uiAudioBytes= 0;
    m_lDurationAud = 0;                                 // In 100NanoSecondsUnits how long a AudioFrames needs for Playback
    m_lSampleTimeAud = 0;                               // Sum of m_lDurationVid, identify the Position of each Frame
    m_pWaveFormat = NULL;
    //FunctionPointer for ClsCoreAudio::ReadBuffer(...)
    m_pAudioData = NULL;
    //m_pReadAudioBuffer;    
    
    // init. all in the same Thread: "Single threaded Apartment"
    HR2(CoInitialize(NULL));                            
    // init. Microsoft Media foundation
    HR2(MFStartup(MF_VERSION));

    //m_myDataForAudioThread.hEventReadAudioHWBuffer = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_myDataForAudioThread.pAudioData = &m_pAudioData;
    m_myDataForAudioThread.uiAudioBytes = &m_uiAudioBytes;
    m_myDataForAudioThread.uiFPS = NULL;
    // Pointer to Pointer (Functionpointer)
    m_myDataForAudioThread.ppReadAudioBuffer = &m_pReadAudioBuffer;
}//END-FUNC
/// <summary>
/// Destructor
/// Reset the Smartpointer
/// Shutdown MediaFoundation
/// </summary>
ClsSinkWriter::~ClsSinkWriter()
{
    m_pBuffer.Reset();
    m_pSample.Reset();
    m_pSinkWriter.Reset();
    MFShutdown();
}//END-FUNC
/*************************************************************************************************
**************************************GENERAL-METHODES********************************************
*************************************************************************************************/
/// <summary>
/// Copying the Frame into the Sample.
/// Video will be builded Sample by Sample.
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsSinkWriter::LoopRecording()
{
    HRESULT hr = NULL;
    unsigned char* pData = m_pFrameData->pData;
    
    if (m_pFrameData->bIsRecording)
    {
        if (*m_pFrameData->pIsAudio)
        {
            WaitForReadAudioHWBuffer();
            HR_RETURN_ON_ERR(hr, WriteAudioDataSample());
        }
        HR_RETURN_ON_ERR(hr, WriteVideoDataSample(pData));
        m_lSampleTimeVid += m_lDurationVid; // Zeit pro Frame in 100NanoSekundenEinheiten
    }
    return hr;
}//END-FUNC
/// <summary>
/// Enables that the Sinkwriter can accept Data
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsSinkWriter::StartRecording()
{
    HRESULT hr = NULL;
    UINT& uiPixelDataSize = m_pFrameData->uiPixelDataSize;
    //if (*m_pFrameData->pIsAudio)
    //{
    //    m_myDataForAudioThread.hEventReadAudioHWBuffer = CreateEvent(NULL, FALSE, FALSE, NULL);
    //}
    // Tell the sink writer to start accepting data.
    HR_RETURN_ON_ERR(hr, m_pSinkWriter->BeginWriting());
    // Create a new memory buffer.
    HR_RETURN_ON_ERR(hr, MFCreateMemoryBuffer(uiPixelDataSize, &m_pBuffer));
    // Set the data length of the buffer.
    HR_RETURN_ON_ERR(hr, m_pBuffer->SetCurrentLength(uiPixelDataSize));

    return hr;
}//END-FUNC
/// <summary>
/// Finish the OutputVideoFile
/// </summary>
/// <returns>HRESULT</returns>
HRESULT ClsSinkWriter::StopRecording()
{
    BOOL bCloseHandle = false;
    HRESULT hr = NULL;

    if (*m_pFrameData->pIsAudio)
    {
        WaitForSingleObject(m_hThreadReadAudioHWBuffer, INFINITE);
        CoreAudio().FinishStream();
    }
    //m_bFinished = true;
    //m_pSinkWriter->Flush(m_dwStreamIndexVidOut);
    //m_pSinkWriter->Flush(m_dwStreamIndexAudOut);
    HR_RETURN_ON_ERR(hr, m_pSinkWriter->Finalize());
    SafeRelease(m_pBuffer.GetAddressOf());
    SafeRelease(m_pSinkWriter.GetAddressOf());

    if (*m_pFrameData->pIsAudio)
    {
        /*bCloseHandle = CloseHandle(m_hThreadReadAudioHWBuffer);
        if (!bCloseHandle)
        {
            printf("close Handle failed: last error is %u\n", GetLastError());
            return E_FAIL;
        }
        else
            m_hThreadReadAudioHWBuffer = NULL;*/
        //bCloseHandle = CloseHandle(m_myDataForAudioThread.hEventReadAudioHWBuffer);
        //if (!bCloseHandle)
        //{
        //    printf("close Handle failed: last error is %u\n", GetLastError());
         //   return E_FAIL;
        //}
        //else
           // m_myDataForAudioThread.hEventReadAudioHWBuffer = NULL;
    }

    m_lSampleTimeVid = 0;
    m_lSampleTimeAud = 0;
    //m_lDurationVid = 0;
    //m_lDurationAud = 0;
    return hr;
}//END-FUNC
/// <summary>
/// Metdaten für IN- und OutputFile: Breite, Höhe, Bitrate, Format etc.
/// </summary>
/// <returns>Bei Fehler HRSEULT-Fehlercode</returns>
HRESULT ClsSinkWriter::PrepareInputOutput()
{
    HRESULT hr = NULL;

    UINT& uiHeightDest = m_pFrameData->uiHeightDest;
    UINT& uiWidthDest = m_pFrameData->uiWidthDest;

    ComPtr<IMFMediaType> pMediaTypeVideoOut;
    ComPtr<IMFMediaType> pMediaTypeAudioOut;
    ComPtr<IMFMediaType> pMediaTypeVideoIn;
    ComPtr<IMFMediaType> pMediaTypeAudioIn;

    IMFAttributes* pAttribute;                          // additionally Attributes for Creatingprocess of the SinkWriter

    if (*m_pFrameData->pIsAudio)
    {
        CoreAudio().PlaySilence();
    }
    MFCreateAttributes(&pAttribute, 2);
    // Disable the DataRateLimit by blocking Thread
    HR_RETURN_ON_ERR(hr, pAttribute->SetUINT32(MF_SINK_WRITER_DISABLE_THROTTLING, true));
    // Enable Hardware Acceleration
    HR_RETURN_ON_ERR(hr, pAttribute->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, true));

    HR_RETURN_ON_ERR(hr, MFCreateSinkWriterFromURL(     // Erstellt Objekt aus IF per Polymorphism
        m_wstrFilename,                                 // Entweder Daten als File speichern
        NULL,                                           // Oder Daten im IMFByteStream speichern
        pAttribute,                                     // IMFAttributes, falls Decoder verwendet werden sollen etc
        &m_pSinkWriter                                  // Pointer des IF das nun auf erstelltes Objekt zeigt
    ));

    SafeRelease(&pAttribute);
    // OUTPUT VIDEO
    HR_RETURN_ON_ERR(hr, MFCreateMediaType(&pMediaTypeVideoOut)); // erstellt ein Obj was ein MedienTyp repräsentiert, Zugriff per IF-Pointer
    // Schlüssel-Wert-Paare
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoOut->SetGUID(MF_MT_SUBTYPE, m_myOutputFormat));
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoOut->SetUINT32(MF_MT_AVG_BITRATE, m_uiBitRate));
    // interlaceMode: progressive jede zeile wird gezeichnet
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    // 1. Pointer wo Attribute des IF gespeichert werden; 2. Welches Attribut; 3. 4. zusamm als 64bit value gespeichert
    HR_RETURN_ON_ERR(hr, MFSetAttributeSize(pMediaTypeVideoOut.Get(), MF_MT_FRAME_SIZE, uiWidthDest, uiHeightDest));
    HR_RETURN_ON_ERR(hr, MFSetAttributeRatio(pMediaTypeVideoOut.Get(), MF_MT_FRAME_RATE, m_pFrameData->uiFPS, 1));
    // Verhältnis des Speicherplatzes  der zuvor gespeicherten Werte: FPS und Zeiteinheit
    // 1:1 heisst 32Bit und 32Bit
    HR_RETURN_ON_ERR(hr, MFSetAttributeRatio(pMediaTypeVideoOut.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1));
    // VideoSpur
    HR_RETURN_ON_ERR(hr, m_pSinkWriter->AddStream(pMediaTypeVideoOut.Get(), &m_dwStreamIndexVidOut));

    if (*m_pFrameData->pIsAudio)
    {
        //OUTPUT Audio mp3
        HR_RETURN_ON_ERR(hr, MFCreateMediaType(&pMediaTypeAudioOut));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_MP3));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_pWaveFormat->nChannels));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_pWaveFormat->nSamplesPerSec));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 192 * 1000 / 8));
        //HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_pWaveFormat->nAvgBytesPerSec));
        //HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 192*1000/8));
        //HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4));
        //HR_RETURN_ON_ERR(hr, pMediaTypeAudioOut->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16));
        HR_RETURN_ON_ERR(hr, m_pSinkWriter->AddStream(pMediaTypeAudioOut.Get(), &m_dwStreamIndexAudOut));
    }
    // INPUT Video
    HR_RETURN_ON_ERR(hr, MFCreateMediaType(&pMediaTypeVideoIn));
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoIn->SetGUID(MF_MT_SUBTYPE, m_myInputFormat));
    HR_RETURN_ON_ERR(hr, pMediaTypeVideoIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    HR_RETURN_ON_ERR(hr, MFSetAttributeSize(pMediaTypeVideoIn.Get(), MF_MT_FRAME_SIZE, uiWidthDest, uiHeightDest));
    HR_RETURN_ON_ERR(hr, MFSetAttributeRatio(pMediaTypeVideoIn.Get(), MF_MT_FRAME_RATE, m_pFrameData->uiFPS, 1));
    HR_RETURN_ON_ERR(hr, MFSetAttributeRatio(pMediaTypeVideoIn.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1));
    HR_RETURN_ON_ERR(hr, m_pSinkWriter->SetInputMediaType(m_dwStreamIndexVidOut, pMediaTypeVideoIn.Get(), NULL));

    if (*m_pFrameData->pIsAudio)
    {
        // INPUT Audio
        HR_RETURN_ON_ERR(hr, MFCreateMediaType(&pMediaTypeAudioIn));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_pWaveFormat->nChannels));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, m_pWaveFormat->wBitsPerSample));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, m_pWaveFormat->nBlockAlign));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_pWaveFormat->nSamplesPerSec));
        HR_RETURN_ON_ERR(hr, pMediaTypeAudioIn->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_pWaveFormat->nAvgBytesPerSec));
        HR_RETURN_ON_ERR(hr, m_pSinkWriter->SetInputMediaType(m_dwStreamIndexAudOut, pMediaTypeAudioIn.Get(), NULL));
    }
    pMediaTypeAudioOut.Reset();
    pMediaTypeVideoOut.Reset();
    pMediaTypeVideoIn.Reset();
    pMediaTypeAudioIn.Reset();
    return hr;
}//END-FUNC
/*************************************************************************************************
**************************************VIDEO-METHODES**********************************************
*************************************************************************************************/
/// <summary>
/// Calculates the Duration of a VideoFrame
/// </summary>
void ClsSinkWriter::CalcDurationVid()
{
    m_lDurationVid = 1000 * 1000 * 10 / m_pFrameData->uiFPS;
    m_myDataForAudioThread.uiFPS = &m_pFrameData->uiFPS;
    //UINT64 b;
    //MFFrameRateToAverageTimePerFrame(m_uiFPS, 1, &b);
    //m_lDurationVid = b;
    // Angabe in  Dauer in 100NanosekundenEinheiten
    // Wieviele 100-NanosekundenEinheiten ein Bild angezeigt werden soll:
    // 30FPS: 1/30 = 0,03Sekunden pro Bild
    // 1000*1000*10/30  = 333.333  100-Nanosekundeneinheiten pro Bild
    // 33.333.333Nanosekunden pro Bild = 0,03 Sekunden pro Bild 
}//END-FUNC
/// <summary>
/// Set the BitRate of the VideoFile for the Sinkwriter
/// </summary>
/// <param name="uiBitRate"></param>
void ClsSinkWriter::SetBitRate()
{
    //m_uiBitRate = uiBitRate;
    m_uiBitRate = m_pFrameData->uiHeightDest * m_pFrameData->uiWidthDest * m_pFrameData->uiBpp;
    m_dwDataRow = m_pFrameData->uiWidthDest * m_pFrameData->uiBpp;
}//END-FUNC
/// <summary>
/// Sets the Bitreading method: Standard or Invert
/// </summary>
/// <param name="myBitReading"></param>
void ClsSinkWriter::SetBitReading(const PicDataBitReading myBitReading)
{
    m_myBitReading = myBitReading;
}//END-FUNC
/// <summary>
/// Set the Name of the VideoFile for the Sinkwriter (char* to WideChar[256])
/// </summary>
/// <param name="strFileName"></param>
void ClsSinkWriter::SetFileName(const char* strFileName)
{
    // const char* to wchar
    size_t uiWritten;
    mbstowcs_s(&uiWritten, m_wstrFilename, strlen(strFileName) + 1, strFileName, strlen(strFileName));
}//END-FUNC
/// <summary>
/// Sets the Input and/or Output Format.
/// When Parameter is NULL, nothing will change
/// </summary>
/// <param name="MyInputFormat">SinkWriter InputFormat</param>
/// <param name="MyOutputFormat">SinkWriter OutputFormat</param>
void ClsSinkWriter::SetFormats(const GUID MyInputFormat, const GUID MyOutputFormat)
{
    m_myInputFormat = MyInputFormat;
    m_myOutputFormat = MyOutputFormat;
}//END-FUNC
/// <summary>
/// Set the FrameDataPointer. Includes Information about the FrameFormat.
/// </summary>
/// <param name="ppFrameData">Adress of the Pointer of the FrameDataStructure</param>
void ClsSinkWriter::SetFrameData(FrameData** ppFrameData)
{
    m_pFrameData = *ppFrameData;
}//END-FUNC
/*************************************/
/// <summary>
/// Return the Duration of one Frame in Milliseconds;
/// </summary>
/// <returns>FrameDuration in Milliseconds</returns>
LONGLONG ClsSinkWriter::GetVideoFrameDuration()
{
    return m_lDurationVid / 10000;
}//END-FUNC
/// <summary>
/// Flips the Frame.
/// BMP-Data have to be readed from down to top.
/// </summary>
/// <param name="dwStride">Width*bpp</param>
/// <param name="pFrameBuffer">Pointer Position Input</param>
/// <param name="pPosition">Pointer Position Output</param>
void ClsSinkWriter::FlipFormat(DWORD& dwStride, const unsigned char* pFrameBuffer, BYTE** pPosition)
{
    PicDataBitReading& myBitReading = m_myBitReading;
    UINT& uiHeightDest = m_pFrameData->uiHeightDest;

    if (myBitReading == PicDataBitReading::Invert)
    {
        // pFrameBuffer ist Position der ersten Zeile
        // mit + (Height*dwStride) haben wir Zeile nach der letzten Zeile
        // deswegen height-1
        *pPosition = (BYTE*)pFrameBuffer + static_cast<UINT64>(uiHeightDest - 1) * dwStride;
        // Mit Angabe eines negativen Elementlänge,
        // sag ich ihm, dass er Buffer rückwärts gehen soll, sonst läuft er vorwärts.
        dwStride = dwStride * (-1);
    }
    else
    {
        *pPosition = (BYTE*)pFrameBuffer;
    }
}//END-FUNC
/// <summary>
/// Aufruf in der Mainloop pro Frame
/// Erstellt ein IMF-Buffer pBuffer.
/// BYTE-Buffer pData wird an IMF-Buffer pBuffer gebunden
/// Kopieren der FrameDaten in pData und somit auch in den IMF-Buffer pBuffer
/// Erstellen eines SinkwriterSamples. pBuffer wird an Sample gebunden
/// Sample wird in den Sinkwriter geschrieben
/// CalledBy: LoopRecording()
/// </summary>
/// <param name="pFrameBuffer">Daten oeines Bildes/Frames</param>
/// <returns></returns>
HRESULT ClsSinkWriter::WriteVideoDataSample(const unsigned char* pFrameBuffer)
{
    HRESULT hr = NULL;
    DWORD dwDataRow =  m_dwDataRow;                 // Referenz auf Wert des Pointers
    DWORD dwStride = m_dwDataRow;
    BYTE* pData = NULL;                             // BYTE-Buffer der an IMF-Buffer gebunden wird
    BYTE* pPosition = NULL;
    UINT& uiPixelDataSize = m_pFrameData->uiPixelDataSize;
    UINT& uiHeightDest = m_pFrameData->uiHeightDest;
   
    FlipFormat(dwStride, pFrameBuffer, &pPosition);
 
    // Lock the buffer and copy the video frame into the buffer.
    HR_RETURN_ON_ERR(hr, m_pBuffer->Lock(&pData, NULL, NULL));
    HR_RETURN_ON_ERR(hr, MFCopyImage(
        pData,                                      // Destination buffer.
        dwDataRow,                                  // Destination stride/ Zeile in Bytes
        pPosition,                                  // PointerPosition
        dwStride,                                   // Zeinenlänge in Bytes mit Vorzeichen (siehe FlipFormat)
        dwDataRow,                                  // Zeinenlänge in Bytes
        uiHeightDest                                // Height in pixels
    ));
    HR_RETURN_ON_ERR(hr, m_pBuffer->Unlock());
    // Set the data length of the buffer.
    HR_RETURN_ON_ERR(hr, m_pBuffer->SetCurrentLength(uiPixelDataSize));
    // Create a media sample and add the buffer to the sample.
    HR_RETURN_ON_ERR(hr, MFCreateSample(&m_pSample));
    // Adds the Buffer with the Data into the Sample
    HR_RETURN_ON_ERR(hr, m_pSample->AddBuffer(m_pBuffer.Get()));
    // Set the time stamp and the duration
    HR_RETURN_ON_ERR(hr, m_pSample->SetSampleTime(m_lSampleTimeVid));
    
    //if (m_bIsAudio)
    //    m_lDurationVid = m_lDurationAud;

    HR_RETURN_ON_ERR(hr, m_pSample->SetSampleDuration(m_lDurationVid));
    // Send the sample to the Sink Writer.
    HR_RETURN_ON_ERR(hr, m_pSinkWriter->WriteSample(m_dwStreamIndexVidOut, m_pSample.Get()));
    return hr;
}//END-FUNC
/*************************************************************************************************
**************************************AUDIO-METHODES**********************************************
*************************************************************************************************/

/// <summary>
/// Preparing Audio Capture
/// - Sets the ReadBuffer Function that is needed in the RecoringLoop ClsD3D11Recording::Recording()
/// - Request the WaveFormat that the Hardwarebuffer will be use.
/// CalledBy: Constructor ClsD3D11Recording::ClsD3D11Recording()
/// </summary>
void ClsSinkWriter::PrepareAudio()
{
    if (*m_pFrameData->pIsAudio)
    {
        SetReadAudioHWBufferCallback(ClsCoreAudio::ReadBuffer);
        CoreAudio().InitAudioClient(WaveFormat());
    }
}//END-FUNC
/// <summary>
/// Enables or disables Audio
/// </summary>
/// <param name="bAudio"></param>
/*void ClsSinkWriter::SetAudio(BOOL bAudio)
{
    m_bIsAudio = bAudio;
}//END-FUNC*/
/// <summary>
/// Starts a seperate Thread for Listening and capturing the HardwareBuffer in Buffer-VectorArray
/// CalledBy: ClsD3D11Recording::Loop()
/// </summary>
HRESULT ClsSinkWriter::StartReadAudioHWBufferThread()
{
    HRESULT hr = NULL;
    DWORD dwThreadID;

    if (*m_pFrameData->pIsAudio && m_pFrameData->bIsRecording)
    {
        m_hThreadReadAudioHWBuffer = CreateThread(
            NULL,
            0,
            AudioHardwareBufferThread,
            &m_myDataForAudioThread,
            0,
            &dwThreadID);
        if (m_hThreadReadAudioHWBuffer == NULL)
        {
            printf("CreateThread failed: last error is %u\n", GetLastError());
            //CloseHandle(m_hThreadReadAudioHWBuffer);
            return E_FAIL;
        }//END-IF
    }
    return hr;
}//END-FUNC
/// <summary>
/// Sets the Functionspointer m_pReadAudioBuffer with the Function of ClsCoreAudio::ReadBuffer(...)
/// </summary>
/// <param name="pReadAudioBuffer">Adress of the Function for Reading the Buffer</param>
void ClsSinkWriter::SetReadAudioHWBufferCallback(HRESULT(*pReadAudioBuffer)(BYTE**, UINT*, UINT*))
{
    m_pReadAudioBuffer = pReadAudioBuffer;
}//END-FUNC
/// <summary>
/// Wait Event for the HardwareBufferThread
/// Waits until its finished
/// CalledBy: ClsSinkWriter::LoopRecording()
/// </summary>
void ClsSinkWriter::WaitForReadAudioHWBuffer()
{
    //WaitForSingleObject(m_myDataForAudioThread.hEventReadAudioHWBuffer, INFINITE);
    WaitForSingleObject(m_hThreadReadAudioHWBuffer, INFINITE);
    CloseHandle(m_hThreadReadAudioHWBuffer);
    m_hThreadReadAudioHWBuffer = NULL;
   // ResetEvent(m_myDataForAudioThread.hEventReadAudioHWBuffer);
}//END-FUNC
/// <summary>
/// Berechnet die Zeit in 100er NanoSekundenEinheiten für n AudioFrames.
/// Erstellt ein AudioSample im SinkWriter.
/// Dieses AudioSample enthält n AudioFrames.
/// CalledBy: ClsCoreAudio::ReadBuffer per FunctionPointer
/// </summary>
/// <param name="pAudioData">AudioData</param>
/// <param name="pNumFrames">Number of AudioFrames (1 Frame = 2 Samples in Stereo)</param>
/// <param name="pWaveFormat">Format Information</param>
/// <returns></returns>
HRESULT ClsSinkWriter::WriteAudioDataSample()
{
    HRESULT hr = NULL;

    ComPtr <IMFSample> pAudioSample;
    ComPtr <IMFMediaBuffer> pAudioBuffer;
    BYTE* pData = NULL;                                    // BYTE-Buffer der an IMF-Buffer gebunden wird

    m_lDurationAud = m_lDurationVid;

    HR_RETURN_ON_ERR(hr, MFCreateMemoryBuffer((DWORD)m_uiAudioBytes, &pAudioBuffer));
    // Bind Normal Buffer to the SinkWriterBuffer
    HR_RETURN_ON_ERR(hr, pAudioBuffer->Lock(&pData, NULL, NULL));
    // Reset Normal Buffer


    memset(pData, 0, m_uiAudioBytes);
    // Write Data into the normal Buffer
    memcpy_s(pData, m_uiAudioBytes, m_pAudioData, m_uiAudioBytes);

    // SinkWriterBuffer has now the Data of the normal Buffer
    HR_RETURN_ON_ERR(hr, pAudioBuffer->Unlock());
    free(m_pAudioData);
    HR_RETURN_ON_ERR(hr, pAudioBuffer->SetCurrentLength((DWORD)m_uiAudioBytes));
    HR_RETURN_ON_ERR(hr, MFCreateSample(&pAudioSample));
    HR_RETURN_ON_ERR(hr, pAudioSample->AddBuffer(pAudioBuffer.Get()));
    //CalcDurationAud(uiNumFrames, pWaveFormat);
    HR_RETURN_ON_ERR(hr, pAudioSample->SetSampleTime(m_lSampleTimeAud));
    HR_RETURN_ON_ERR(hr, pAudioSample->SetSampleDuration(m_lDurationAud));
    m_lSampleTimeAud = m_lDurationAud + m_lSampleTimeAud;
    HR_RETURN_ON_ERR(hr, m_pSinkWriter->WriteSample(m_dwStreamIndexAudOut, pAudioSample.Get()));

    return hr;
}//END-FUNC
/// <summary>
/// - Static Function, called by the starting Thread m_hThreadReadAudioHWBuffer
/// - Calls ClsCorAudio::ReadBuffer(...)
/// CalledBy: ClsSinkWriter::StartReadAudioHWBufferThread()
/// </summary>
/// <param name="pParm"></param>
/// <returns>always 0</returns>
DWORD WINAPI ClsSinkWriter::AudioHardwareBufferThread(LPVOID pParm) {
    DataForAudioThread* pDataForAudioThread;
    pDataForAudioThread = (DataForAudioThread*)pParm; // unzip void Pointer

    // Call the Function in the FunctionPoiner: ClsAudioCore::Readbuffer()
    (*pDataForAudioThread->ppReadAudioBuffer)(pDataForAudioThread->pAudioData, pDataForAudioThread->uiAudioBytes, pDataForAudioThread->uiFPS);
    //SetEvent(pDataForAudioThread->hEventReadAudioHWBuffer);
    return 0;
}//END-FUNC
/// <summary>
/// returns the whole CoreAudioObject
/// CalledBy: Constructor, StopReacording(), PrepareInputOutput()
/// </summary>
/// <returns>CoreAudioObject</returns>
ClsCoreAudio& ClsSinkWriter::CoreAudio()
{
    return m_myClsCoreAudio;
}//END-FUNC
/// <summary>
/// Used by CoreAudio for setting the WaveFormat
/// CalledBy: CoreAudio().InitAudioClient(WaveFormat()) in the Constructor
/// </summary>
/// <returns>a set WaveFormat</returns>
WAVEFORMATEX** ClsSinkWriter::WaveFormat() 
{ 
    return &m_pWaveFormat; 
}//END-FUNC
