#pragma once
#include "framework.h"
/// <summary>
/// used in Callback ClsWinGDI::EnumWindowsProc as lParam
/// Callback is called in EnumWindowProc
/// keeps the specific WndHandle, depending ont the Title
/// </summary>
namespace GDI
{
	class ClsWndHandle
	{
	public:
		friend class ClsWinGDI;
		friend class ClsCalcD3DWnd;
		//friend class FrameData;
	private:
		BOOL m_bIsWnd;
		HWND m_hWnd;
		char* m_strTitle;
		DWORD m_dwPid;
	public:
		ClsWndHandle();
	private:
		void SetHandle(HWND hWnd);
		void SetWndTitle(const char* strTitle);
		BOOL CheckWnd();
		HWND GetHandle();
		char* GetWndTitle();
	};
}