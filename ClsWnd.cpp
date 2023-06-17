#pragma once

#include "ClsWnd.h"
#include <sstream>

/*
void ClsWnd::SetWndProp()
{
	m_MyWndProp.SetRes();
}
WndProperty& ClsWnd::GetWndProp()
{
	return m_MyWndProp;
}
*/

/// <summary>
/// Konstruktor von ClsWnd
/// Hier wird das registrierte Fenster erstellt
/// (Registriert/eingetragen in ClsWndProc, Ident per  2. Param von CreateWindowEx)
/// -> Er muss ja wissen welches registrierte Fenster erstellt werden soll
/// </summary>
/// <param name="pstrName">TitleBar Name</param>
/// <param name="uiXPosition">StartPosition Y</param>
/// <param name="uiYPosition">StartPosition X</param>
/// <param name="uiWidth">Breite</param>
/// <param name="uiHeight">Höhe</param>
ClsWnd::ClsWnd(LPCWSTR pstrName, ClsD3D11Recording* pClsD3D11Recording)
{
	//RECT struDrawingSize;

	//SetWndProp();

	m_hInstance = GetModuleHandle(NULL);
	m_pClsD3D11Recording = pClsD3D11Recording;
	//struDrawingSize.top = 0;
	//struDrawingSize.bottom = uiHeight;
	//struDrawingSize.left = 0;
	//struDrawingSize.right = uiHeight;

	//AdjustWindowRect(&GetWndProp().GetWndRect(), WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
	//m_pClsDataContainer->
	m_hWnd = CreateWindowEx(
		CS_OWNDC,					// Style Flags
		ClsWndProc::GetMyClassName(),
		pstrName,					// Name in Titelbar
		m_iMyFlags,					// Flags wie das Window sich verhält
		m_pClsD3D11Recording->GetXPos(), // GetWndProp().GetXPos(),				// Start X
		m_pClsD3D11Recording->GetYPos(), //GetWndProp().GetYPos(),				// Start Y	
		m_pClsD3D11Recording->GetWndXSize(),					// Width
		m_pClsD3D11Recording->GetWndYSize(),					// Height
		NULL,						// ParentWindow
		NULL,						// Menu
		m_hInstance,		// Current hInstance 
		this						// CustomParameter, VoidPointer
	);

	m_pClsD3D11Recording->SetHWND(m_hWnd);
	CreateMyMenu();
}//END-CONSTR

void ClsWnd::CreateMyMenu()
{
	ClsMenu oMyMenu(m_hWnd);
}
BOOL ClsWnd::RunMsgLoop() {
	// Stop and return false if window is not valid
	if (!m_hWnd)
		return FALSE;
	/* PeekMessage:
	* Liest die Msgs aus die vom Fenster gesendet werden
	* PARAM: 
	* 1. Speicher für zu lesende Msg
	* 2. Von welchen Fenster soll Msg empfangen werden
	* 3. MsgFilter von
	* 4. MsgFilter bis (Flags um Msgs zu Filtern, damit wir nich talle bekommen, 0 = kein Filter)
	* 5. Was soll mit Msg gemacht werden nachdem wir sie gelesen haben		
	*/
	while (PeekMessage(&m_Msg, m_hWnd, 0, 0, PM_REMOVE)) 
	{
		TranslateMessage(&m_Msg);	// setzt/fügt interne Param dazu
		DispatchMessage(&m_Msg);	// ruft unsere Callback-Funktion auf 
									// die wir im Wnd-Descriptor angegeben haben (ClsWndProc::MsgProcSetup/MsgProcRun)
	}
	return TRUE;
}//END-RunMessageLoop

/// <summary>
/// Show or hide the Window
/// </summary>
/// <param name="bVisible">True: Show</param>
/// <returns>True if success</returns>
BOOL ClsWnd::SetVisibility(BOOL bVisible) 
{
	// Return false if handle is not valid
	if (m_hWnd == NULL)
		return FALSE;

	// Show / Hide
	if (bVisible)
		return ShowWindow(m_hWnd, SW_SHOW);
	else
		return ShowWindow(m_hWnd, SW_HIDE);
}//END-FUNC

/// <summary>
/// Msg-Handling
/// CalledBy: ClsWndProc::MsgProcRun(...)
/// </summary>
/// <param name="wnd"></param>
/// <param name="msg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT ClsWnd::ProcessTriggeredMsg(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		// Window destroyd
	case WM_DESTROY:
	{
		// Destroy window and retur own handled
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		return NULL;
	}
	break;

	// Window key down
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		// Set bit 
		m_keyState.set((UCHAR)wParam);
		return NULL;
	}
	break;

	// Window key up
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		// Clear bit
		m_keyState.reset((UCHAR)wParam);
		return NULL;
	}
	break;

	// Window in focus
	case WM_SETFOCUS:
	{
		// Window in focus
		m_bInFocus = TRUE;
	}
	break;

	// Window out of focus
	case WM_KILLFOCUS:
	{
		// No Foucs
		m_bInFocus = FALSE;
		// Clear all bits
		m_keyState.reset();
	}
	break;


	// Mouse Keys
	case WM_LBUTTONDOWN:
	{
		m_mouseState.lKeyDown = TRUE;
	}
	break;
	case WM_MBUTTONDOWN:
	{
		m_mouseState.mKeyDown = TRUE;
	}
	break;
	case WM_RBUTTONDOWN:
	{
		m_mouseState.rKeyDown = TRUE;
	}
	break;
	case WM_LBUTTONUP:
	{
		m_mouseState.lKeyDown = FALSE;
	}
	break;
	case WM_MBUTTONUP:
	{
		m_mouseState.mKeyDown = FALSE;
	}
	break;
	case WM_RBUTTONUP:
	{
		m_mouseState.rKeyDown = FALSE;
	}
	break;

	// Mouse move
	case WM_MOUSEMOVE:
	{
		m_mouseState.posX = GET_X_LPARAM(lParam);
		m_mouseState.posY = GET_Y_LPARAM(lParam);
	}
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Menüauswahl analysieren:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hWnd, &ClsWnd::About);
			break;
		case IDM_EXIT:
			DestroyWindow(m_hWnd);
			break;
		default:
			return DefWindowProc(m_hWnd, msg, wParam, lParam);
		}
	}
	break;

	// ...


	}

	// Fallback default window proc
	return DefWindowProc(wnd, msg, wParam, lParam);
}

// Meldungshandler für Infofeld.
INT_PTR CALLBACK ClsWnd::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}