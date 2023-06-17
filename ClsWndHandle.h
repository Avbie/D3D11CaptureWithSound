#pragma once
#include "framework.h"
/// <summary>
/// Wird als lParm in der EnumWindows (Auflistung aller Fenster) genutzt:
///	Aufruf der EnumWindowsProc in EnumWindows in loop.
///	EnumWindowsProc wurde von uns überschrieben.
///	Zugriff auf lParm der auf MyHandle gecastet wird.
///	Zugriff in der WindowsFunktion auf unsere MemberVariable von  der Klasse ClsBitBlt.
/// Beinhaltet gesuchten WindowsTitle
/// </summary>
class ClsWndHandle
{
private:
	BOOL m_bIsWnd;
	HWND m_hWnd;
	char m_strTitle[200] = "\0";
	DWORD m_dwPid;
public:
	ClsWndHandle()
	{
		m_bIsWnd = false;
		m_hWnd = NULL;
		m_dwPid = 0;
	}
public:
	HWND GetHandle()
	{
		return m_hWnd;
	}
	void SetHandle(HWND hWnd)
	{
		m_hWnd = hWnd;
		GetWindowThreadProcessId(m_hWnd, &m_dwPid);
		m_bIsWnd = true;
	}
	char* GetWndTitle()
	{
		return m_strTitle;
	}
	BOOL CheckWnd()
	{
		if (IsWindow(m_hWnd))
		{
			DWORD dwNewId = NULL;
			GetWindowThreadProcessId(m_hWnd, &dwNewId);
			if (m_dwPid == dwNewId)
			{
				m_bIsWnd = true;
				return true;
			}
			else
			{
				m_hWnd = NULL;
				m_dwPid = 0;
				m_bIsWnd = false;
				return false;
			}
		}
		else
		{
			m_hWnd = NULL;
			m_dwPid = 0;
			m_bIsWnd = false;
			return false;
		}
	}

	void SetWndTitle(const char* strTitle)
	{
		strcpy_s(m_strTitle, strTitle);
	}
};