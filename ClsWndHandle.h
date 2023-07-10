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
		ActiveWnd()
		{
			hWnd = NULL;
			sTitle = "";
			dwProcessID = -1;
		}
	};
	class ClsWndHandle
	{
	public:
		friend class ClsWinGDI;
		friend class ClsCalcD3DWnd;
	private:
		ActiveWnd m_myActiveWnd;
	public:
		ClsWndHandle();
		HWND GetWndHandle();
	private:
		void SetActiveWnd(const ActiveWnd myActivWnd);
		const DWORD GetProcessID();
	};
}