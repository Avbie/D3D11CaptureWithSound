#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <bitset>

#include "Resource.h"
#include "ClsWndProc.h"
#include "ClsD3D11Recording.h"
#include "ClsMenu.h"
//todo setres/getres brauchen wir nicht mehr, wird �ber d3d ermittelt, alles raus
//uiWidth = 1680; 2560; 1920; oMyWnd.GetResolution().GetWidth();
	//uiHeight = 1050;//1440;// 1080;// oMyWnd.GetResolution().GetHeight();
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
public:
	ClsWnd(LPCWSTR pstrName, ClsD3D11Recording* pClsD3D11Recording);
	BOOL SetVisibility(BOOL bVisible);
	BOOL RunMsgLoop();
	LRESULT ProcessTriggeredMsg(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND GetHWND() { return m_hWnd; }
private:
	void CreateMyMenu();
	//void SetWndProp();
	//WndProperty& GetWndProp();
public:
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	ClsD3D11Recording* m_pClsD3D11Recording;
	//WndProperty m_MyWndProp;
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	MSG m_Msg = {};
	int m_iMyFlags = WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	BOOL m_bInFocus = FALSE;
	std::bitset<256> m_keyState;
	MouseState m_mouseState = {};
};