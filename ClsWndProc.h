#pragma once
#include <Windows.h>

#include "ClsWnd.h"

class ClsWndProc
{
public:
	static LRESULT CALLBACK MsgProcSetup(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MsgProcRun(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LPCWSTR GetMyClassName();
private:
	ClsWndProc();
	~ClsWndProc();
	//return ClsWnd
private:
	HINSTANCE m_hInstance;
	static ClsWndProc SingletonInstance;
	static LPCWSTR m_lpClassName;
};