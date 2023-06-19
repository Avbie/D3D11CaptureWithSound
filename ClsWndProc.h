#pragma once
#include <Windows.h>
#include "ClsWnd.h"

/// <summary>
/// Singleton Class
/// will be executed when app is running
/// Register a Windowclass with the specific longPointerFunction lpfnWndProc
/// registered class/Window will be used by CreateWindowEx in ClsWnd::ClsWnd
/// </summary>
class ClsWndProc
{
public:
	static LRESULT CALLBACK MsgProcSetup(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MsgProcRun(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LPCWSTR GetMyClassName();
private:
	ClsWndProc();
	~ClsWndProc();
private:
	HINSTANCE m_hInstance;
	static ClsWndProc SingletonInstance;
	static LPCWSTR m_lpClassName;
};