#pragma once
#include "framework.h"
#include "ClsWndHandle.h"

class ClsWndCallback
{
public:
	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {

		TCHAR buf[1024]{};
		TCHAR strName[1024]{};
		USES_CONVERSION;

		ClsWndHandle* pClsWndHandle = reinterpret_cast<ClsWndHandle*>(lParam);
		GetClassName(hWnd, buf, 100);
		GetWindowText(hWnd, strName, 1024);
		if (_tcsstr(strName, A2T(pClsWndHandle->GetWndTitle())))		// "atlstr.h"  A2T-Macro  ANSI to Unicode
		{
			pClsWndHandle->SetHandle(hWnd);
			return FALSE;
		}
		return TRUE;
	}
};
