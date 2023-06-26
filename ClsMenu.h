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
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileM, L"File");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hRecord, L"Capture");
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hVidSrc, L"Pick Monitor");
		AppendMenu(hFileM, MF_STRING, 1, L"New");
		AppendMenu(hFileM, MF_STRING, IDM_ABOUT, L"About");
		AppendMenu(hRecord, MF_STRING, IDM_RECORDSTART, L"Start");
		AppendMenu(hRecord, MF_STRING, IDM_RECORDSTOP, L"Stop");
		AppendMenu(hVidSrc, MF_STRING, IDM_MONMAIN, L"MainMonitor");
		AppendMenu(hVidSrc, MF_STRING, IDM_MON1, L"Monitor 1");
		AppendMenu(hVidSrc, MF_STRING, IDM_WINDOW, L"Window");
		SetMenu(hWnd, hMenu);
	}
	friend class ClsWnd;
};