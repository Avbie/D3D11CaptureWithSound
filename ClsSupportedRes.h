#pragma once

#include <iostream>
#include <vector>
#include <algorithm>    // std::sort
#include <Windows.h>
#include <string>

#include "framework.h"


struct SupportedResolution
{
public:
    friend class Compare;
    friend class FindIf;
    friend class ClsSupportedRes;
private:
    DWORD dwWidth;
    DWORD dwHeight;
    std::string sDescription;
public:
    SupportedResolution()
    {
        dwWidth = 0;
        dwHeight = 0;
        sDescription = "";
    }
    const DWORD GetHeight()
    {
        return dwHeight;
    }
    const DWORD GetWidth()
    {
        return dwWidth;
    }
};

/// <summary>
/// Used in ClsSupportedRes as static variable m_myCompare.
/// This variable is used as compare function as callback function in
/// std::sort(m_vSupportedRes.begin(), m_vSupportedRes.end(), ClsSupportedRes::m_myCompare);
/// We need a deep compare here. We cant compare the struct SupportedResolution itself, 
/// instead we compare the membervaribales of the struct SupportedResolution.
/// The operator() is overwritten here. It overwrites the call of the Object itself with 2 Parameters
/// It returns true or false.
/// </summary>
class Compare
{
public:
    bool operator() (SupportedResolution& myRes1, SupportedResolution& myRes2)
    {
        return (myRes1.dwWidth < myRes2.dwWidth);
    }
};

class ClsFind
{
public:
    DWORD m_dwWidth;
    DWORD m_dwHeight;
    ClsFind(const DWORD dwWidth, const DWORD dwHeight)
    {
        m_dwWidth = dwWidth;
        m_dwHeight = dwHeight;
    }
public:
    bool operator() (SupportedResolution& otherRes)
    {
        if (m_dwWidth == otherRes.GetWidth() && m_dwHeight == otherRes.GetHeight())
            return TRUE;
        return FALSE;
    }
};

class ClsSupportedRes
{
private:
    const wchar_t* m_strDevice;
    static Compare m_myCompare;
    std::vector<SupportedResolution> m_vSupportedRes;
    static std::vector<SupportedResolution> m_vSupportedResOfAllMon;  
public:
    ClsSupportedRes(const wchar_t* strDevice);
public:
    static void GetDefaultResolution(DWORD* pWidth, DWORD* pHeight);
    static BOOL SelectDestResolution(const int iResIndexNr, DWORD* pWidth, DWORD* pHeight);
    static INT64 GetIndexOfSelectedRes(DWORD dwWidth, DWORD dwHeight);
    static const std::vector<SupportedResolution>& GetAllSupportedRes();
    const std::vector<SupportedResolution>& GetSupportedRes();
private:
    void CreateListOfAllSupportedRes();
    void RequestSupportedResolutions();
    static BOOL Duplicates(const SupportedResolution& myRes1, const SupportedResolution& myRes2);
};