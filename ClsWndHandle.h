#pragma once
#include "framework.h"

using namespace std;
/// <summary>
/// used in Callback ClsWinGDI::EnumWindowsProc as lParam
/// Callback is called in EnumWindowProc
/// keeps the specific WndHandle, depending ont the Title
/// </summary>
namespace GDI
{
	struct ActiveWnd
	{
		HWND hWnd;
		string sTitle;
		DWORD dwProcessID;
	};
	class ClsWndHandle
	{
	public:
		friend class ClsWinGDI;
		friend class ClsCalcD3DWnd;
	private:
		ActiveWnd m_myActiveWnd;
		BOOL m_bIsWndSet;
	public:
		ClsWndHandle();
	private:
		void SetDesktop();
		void SetActiveWnd(ActiveWnd myActivWnd);
		BOOL IsWndSet();
		HWND GetHandle();
		DWORD GetProcessID();
	};
}