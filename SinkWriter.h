#pragma once

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
/*
template <class T> 
void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
void PrepareSinkWriter();
void ProgressingSinkWriter(unsigned char* pFrame);
void FinalizeSinkWriter();*/