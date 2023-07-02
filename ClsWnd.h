#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <bitset>

#include "Resource.h"
#include "ClsWndProc.h"
#include "ClsD3D11Recording.h"
#include "ClsMenu.h"

struct MouseState {
	BOOL lKeyDown, mKeyDown, rKeyDown;
	UINT posX, posY;
};
/// <summary>
/// Klasse um ein Fenster zu erstellen, dazu wird der Descriptor in der ClsWndProc ben�tigt
/// Fenster wird im Konstruktor erstellt
/// </summary>
class ClsWnd
{
private:
	BOOL m_bInFocus;
	
	HINSTANCE m_hInstance;
	//static UINT m_uiEditControlFPS;
	UINT m_uiMyFlags;
	std::bitset<256> m_keyState;
	MouseState m_mouseState;
	MSG m_Msg;
	static ClsD3D11Recording* m_pClsD3D11Recording;
public:
	static HWND m_hDlgSettings;
	HWND m_hWnd;
public:
	ClsWnd(LPCWSTR pstrName, ClsD3D11Recording* pClsD3D11Recording);
public:
	BOOL SetVisibility(BOOL bVisible);
	BOOL RunMsgLoop();
	HWND GetHWND();
	LRESULT ProcessTriggeredMsg(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR  CALLBACK Settings(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void CreateMyMenu();
	void CreateButton();
};