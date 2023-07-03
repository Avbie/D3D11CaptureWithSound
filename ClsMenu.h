#pragma once
#include <Windows.h>

class ClsMenu
{
public:
	ClsMenu(HWND hWnd)
	{
		HMENU hMenu = CreateMenu();
		HMENU hFileM = CreateMenu();
		HMENU hRecord = CreateMenu();
		HMENU hVidSrc = CreateMenu();
		HMENU hSettings = CreateMenu();
		HMENU hWndSize = CreateMenu();
		HMENU hSetWnd = CreateMenu();
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileM, L"File");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hRecord, L"Capture");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hVidSrc, L"Pick Monitor");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSettings, L"Settings");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hWndSize, L"Fenstergröße zur Quelle");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSetWnd, L"Actives Fenster als Quelle");
		AppendMenu(hFileM, MF_STRING, 1, L"New");
		AppendMenu(hFileM, MF_STRING, IDM_ABOUT, L"About");
		AppendMenu(hRecord, MF_STRING, IDM_RECORDSTART, L"Start");
		AppendMenu(hRecord, MF_STRING, IDM_RECORDSTOP, L"Stop");
		AppendMenu(hVidSrc, MF_STRING, IDM_MONMAIN, L"MainMonitor");
		AppendMenu(hVidSrc, MF_STRING, IDM_MON1, L"Monitor 1");
		AppendMenu(hVidSrc, MF_STRING, IDM_WINDOW, L"Window");
		AppendMenu(hSettings, MF_STRING, IDM_SETTINGS, L"Settings");

		AppendMenu(hWndSize, MF_STRING, IDM_WNDSIZE1, L"150%");
		AppendMenu(hWndSize, MF_STRING, IDM_WNDSIZE2, L"100%");
		AppendMenu(hWndSize, MF_STRING, IDM_WNDSIZE3, L"50%");

		AppendMenu(hSetWnd, MF_STRING, IDM_WNDSET1, L"Fenster 1");
		AppendMenu(hSetWnd, MF_STRING, IDM_WNDSET2, L"Fenster 2");
		AppendMenu(hSetWnd, MF_STRING, IDM_WNDSET3, L"Fenster 3");
		SetMenu(hWnd, hMenu);
	}
	friend class ClsWnd;
};