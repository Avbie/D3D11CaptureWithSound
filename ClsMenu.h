#pragma once
#include <Windows.h>

class ClsMenu
{
public:
	ClsMenu(HWND hWnd)
	{
		HMENU hMenu = CreateMenu();
		HMENU hFileM = CreateMenu();
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileM, L"File");
		AppendMenu(hFileM, MF_STRING, 1, L"New");
		AppendMenu(hFileM, MF_STRING, IDM_ABOUT, L"About");
		SetMenu(hWnd, hMenu);
	}
	friend class ClsWnd;
};