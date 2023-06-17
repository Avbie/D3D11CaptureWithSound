#include "SinkWriter.h"
/*
// Format constants
const UINT32 VIDEO_WIDTH = 2560;
const UINT32 VIDEO_HEIGHT = 1440;
const UINT32 VIDEO_FPS = 60;                     // 1/5 = 0,2 sek
const UINT64 VIDEO_FRAME_DURATION = 166666; // = (0,2 Sek*1000^3)/100 = 100NanosekundenEinheiten pro Bild; 0,2*  10 * 1000 * 1000 / VIDEO_FPS;
const UINT32 VIDEO_BIT_RATE = 4000000;// 2560 * 1440 * 32 * 165; Bitrate pro sek
const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_H264; //MFVideoFormat_ARGB32; // MFVideoFormat_WMV3;
const GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_ARGB32; // MFVideoFormat_RGB32;
const UINT32 VIDEO_PELS = VIDEO_WIDTH * VIDEO_HEIGHT;
const UINT32 VIDEO_FRAME_COUNT = 20 * VIDEO_FPS;

int iFunctionCalls = 0;
bool bFinished = 0;
LONGLONG rtStart = 0;
IMFSinkWriter* pSinkWriter = NULL; // Da um Mehrere Buffer bzw ein Sample in File zu speichern
                                               // es können mehrere Samples hintereinander sein
DWORD stream;                      // Index
//DWORD videoFrameBuffer[VIDEO_PELS];

// Metadaten beschreibung für Input und Output File
HRESULT InitializeSinkWriter(IMFSinkWriter** ppWriter, DWORD* pStreamIndex)
{
    *ppWriter = NULL;
    *pStreamIndex = NULL;

    IMFSinkWriter* pSinkWriter = NULL;
    IMFMediaType* pMediaTypeOut = NULL;
    IMFMediaType* pMediaTypeIn = NULL;
    DWORD           streamIndex;

    // Erstellung eines SinkWriter
    HRESULT hr = MFCreateSinkWriterFromURL(     // Erstellt Objekt aus IF per Polymorphism
        L"output.mp4",                          // Entweder Daten als File speichern
        NULL,                                   // Oder Daten im IMFByteStream speichern
        NULL,                                   // IMFAttributes, falls Decoder verwendet werden sollen etc
        &pSinkWriter                            // Pointer des IF das nun auf erstelltes Objekt zeigt
    );

    // Set the output media type.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateMediaType(&pMediaTypeOut); // erstellt ein Obj was ein MedienTyp repräsentiert, Zugriff per IF-Pointer
    }
    if (SUCCEEDED(hr))
    {
        hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        // Zuweisung von Schlüssel-Wert-Paaren
        // Ordnet einen 
        // 1. Param: Key. Hier HauptMedienTyp
        // 2. Param: Wert. Hier MFMediaType_Video,  kann auch audio sein
        // GIUD ist eine Struktur aus: ul, us, us und unsigned char array
    }
    if (SUCCEEDED(hr))
    {
        hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, VIDEO_ENCODING_FORMAT);
        // Subtyp innerhalb des Haupttypes
        // VideoFormat, hier MFVideoFormat_WMV3
    }
    if (SUCCEEDED(hr))
    {
        // Bitrate
        hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_BIT_RATE);
    }
    if (SUCCEEDED(hr))
    {
        hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
        // interlaceMode: progressive jede zeile wird gezeichnet
    }
    //if (VIDEO_ENCODING_FORMAT == MFVideoFormat_H264)
    //    pMediaTypeOut->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_Wide);

    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
        // 1. Pointer wo Attribute des IF gespeichert werden
        // 2. Welches Attribut
        // 3. 4.  werden zusamm als 64bit value gespeichert
        // Höhe und breite
    }
    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
        // 1. Pointer wo Attribute des IF gespeichert werden
        // 2. Welches Attribut
        // 3. Anzahl der Bilder
        // 4. Zeiteinheit in Sekunden
        // 3. und 4.  werden zusamm als 64bit value gespeichert (2. Param gibt speicherziel an)

    }
    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
        // verhältnis des Speicherplatzes  der zuvor gespeicherten Werte: FPS und Zeiteinheit
        // 1:1 heisst 32Bit und 32Bit

    }
    if (SUCCEEDED(hr))
    {
        hr = pSinkWriter->AddStream(pMediaTypeOut, &streamIndex);
        // Fügt den Stream zum sinkwriter hinzu
        // 1. IF-Pointer, der OutputFile beschreibt
        // 2. Index
    }

    // selbe spiel für die Input daten
    // Set the input media type.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateMediaType(&pMediaTypeIn);
    }
    if (SUCCEEDED(hr))
    {
        hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    }
    if (SUCCEEDED(hr))
    {
        hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, VIDEO_INPUT_FORMAT);
    }
    if (SUCCEEDED(hr))
    {
        hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    }


    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
    }
    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
    }
    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);
    }

    // Tell the sink writer to start accepting data.
    if (SUCCEEDED(hr))
    {
        hr = pSinkWriter->BeginWriting();
    }

    // Return the pointer to the caller.
    if (SUCCEEDED(hr))
    {
        *ppWriter = pSinkWriter;
        (*ppWriter)->AddRef();
        *pStreamIndex = streamIndex;
    }

    SafeRelease(&pSinkWriter);
    SafeRelease(&pMediaTypeOut);
    SafeRelease(&pMediaTypeIn);
    return hr;
}

HRESULT WriteFrame(
    IMFSinkWriter* pWriter,
    DWORD streamIndex,
    const LONGLONG& rtStart,        // Time stamp.
    unsigned char* pFrameBuffer
)
{
    IMFSample* pSample = NULL;
    IMFMediaBuffer* pBuffer = NULL;

    const LONG cbWidth = 4 * VIDEO_WIDTH;
    const DWORD cbBuffer = cbWidth * VIDEO_HEIGHT;

    BYTE* pData = NULL;

    // Create a new memory buffer.
    HRESULT hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

    // Lock the buffer and copy the video frame to the buffer.
    if (SUCCEEDED(hr))
    {
        hr = pBuffer->Lock(&pData, NULL, NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = MFCopyImage(
            pData,                      // Destination buffer.
            cbWidth,                    // Destination stride.
            // pFrameBuffer ist Position der ersten Zeile
            // mit + (Height*width) haben wir Zeile nach der letzten Zeile
            // deswegen height-1
            (BYTE*)pFrameBuffer + (VIDEO_HEIGHT -1) * cbWidth, 
            
            // Mit Angabe eines Negativen Elementlänge (hier eine Zeile)
            // sag ich ihm das er Buffer rückwärts gehen soll, sonst läuft er vorwärts
            // und somit ausserhalb des speichers für die pixeldaten
            cbWidth*-1,                 // Source stride.
            cbWidth,                    // Image width in bytes.
            VIDEO_HEIGHT                // Image height in pixels.
        );
    }
    if (pBuffer)
    {
        pBuffer->Unlock();
    }

    // Set the data length of the buffer.
    if (SUCCEEDED(hr))
    {
        hr = pBuffer->SetCurrentLength(cbBuffer);
    }

    // Create a media sample and add the buffer to the sample.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateSample(&pSample);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSample->AddBuffer(pBuffer);
    }

    // Set the time stamp and the duration.
    if (SUCCEEDED(hr))
    {
        hr = pSample->SetSampleTime(rtStart);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSample->SetSampleDuration(VIDEO_FRAME_DURATION);
        // Angabe in  Dauer in 100NanosekundenEinheiten
        // 30FPS: 1/30 = 0,03Sekunden pro Bild
        // 1000*1000*10/30  = 333.333  100-Nanosekundeneinheiten pro Bild
        // 33.333.333Nanosekunden pro Bild = 0,03 Sekunden pro Bild 
    }

    // Send the sample to the Sink Writer.
    if (SUCCEEDED(hr))
    {
        hr = pWriter->WriteSample(streamIndex, pSample);
    }

    SafeRelease(&pSample);
    SafeRelease(&pBuffer);
    return hr;
}


void PrepareSinkWriter()
{
    // Set all pixels to green
  /*  for (DWORD i = 0; i < VIDEO_PELS; ++i)
    {
        videoFrameBuffer[i] = 0x0000FF00;
    }*/
/*
    HRESULT hr = CoInitialize(NULL);  //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    // muss aufgerufen werden um COM+ verwenden zu können
    // mehrere threads mgl, aber mit COINIT_APARTMENTTHREADED: Erstellung und Ausführung immer im gleichen thread 
    // -> keine Absprache/Parallelisierung notwendig
    if (!SUCCEEDED(hr))
        return;
    hr = MFStartup(MF_VERSION);     // init Microsoft Media foundation ( beinhalted SinkWriter etc)
    if (!SUCCEEDED(hr))
        return;
    
   // IMFSinkWriter* pSinkWriter = NULL; // Da um Mehrere Buffer bzw ein Sample in File zu speichern
                                               // es können mehrere Samples hintereinander sein
    //DWORD stream;                      // Index

    hr = InitializeSinkWriter(&pSinkWriter, &stream);
    if (!SUCCEEDED(hr))
        return;
     // Send frames to the sink writer.
    //LONGLONG rtStart = 0;


    /*for (DWORD i = 0; i < VIDEO_FRAME_COUNT; ++i)
    {
        hr = WriteFrame(pSinkWriter, stream, rtStart);
        if (FAILED(hr))
        {
            break;
        }
        rtStart += VIDEO_FRAME_DURATION; // Zeit pro Frame in 100NanoSekundenEinheiten
        // Muss passend mit ergebnis von 1/BilderProSek  sein
    }*/
   
    //hr = pSinkWriter->Finalize();
          
    //SafeRelease(&pSinkWriter);
    //MFShutdown();
       
   // CoUninitialize();
  /*
}

void ProgressingSinkWriter(unsigned char* pFrame)
{
    if (bFinished)
        return;
 
    HRESULT hr = WriteFrame(pSinkWriter, stream, rtStart, pFrame);
    if (FAILED(hr))
    {
        return;
    }
    rtStart += VIDEO_FRAME_DURATION; // Zeit pro Frame in 100NanoSekundenEinheiten
    // Muss passend mit ergebnis von 1/BilderProSek  sein
    
    if (iFunctionCalls == VIDEO_FRAME_COUNT)
        FinalizeSinkWriter();
    else
        iFunctionCalls++;
       

}
void FinalizeSinkWriter()
{
    bFinished = 1;
    HRESULT hr = pSinkWriter->Finalize();

    SafeRelease(&pSinkWriter);
    MFShutdown();

    CoUninitialize();
}

void RGB32_To_AYUV(
    BYTE* pDest,
    const BYTE* pSrc,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
)
{
    
    for (DWORD y = 0; y < dwHeightInPixels; y++)
    {
        RGBQUAD* pSrcPixel = (RGBQUAD*)pSrc;
        RGBQUAD* pDestPixel = (RGBQUAD*)pDest;

        for (DWORD x = 0; x < dwWidthInPixels; x++)
        {
            pDestPixel[x] = pSrcPixel[x];
        }
        pDest += 4;
        pSrc += 4;
    }
}*/