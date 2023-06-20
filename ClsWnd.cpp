#pragma once
#include "ClsWnd.h"
/// <summary>
/// Konstruktor von ClsWnd
/// Creation of the registered Window
/// (Registriert/eingetragen in ClsWndProc, Ident per  2. Param von CreateWindowEx)
/// - CreateWindowEx triggers the function ClsWndProc::MsgProcSetup(...) 
///   that will be set during the register-process
/// - custom Pointer "this" of our ClsWnd will be transfered to ClsWndProc::MsgProcSetup/MsgProcRun(...)
///   - ClsWndProc::MsgProcRun(...) calls ClsWnd::ProcessTriggeredMsg(...)
/// </summary>
/// <param name="pstrName">TitleBar Name</param>
/// <param name="uiXPosition">StartPosition Y</param>
/// <param name="uiYPosition">StartPosition X</param>
/// <param name="uiWidth">Width</param>
/// <param name="uiHeight">Height</param>
ClsWnd::ClsWnd(LPCWSTR pstrName, ClsD3D11Recording* pClsD3D11Recording)
{
	m_bInFocus = FALSE;
	m_hInstance = GetModuleHandle(NULL);
	m_uiMyFlags = WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	m_mouseState = {};
	m_Msg = {};
	m_pClsD3D11Recording = pClsD3D11Recording;

	m_hWnd = CreateWindowEx(
		CS_OWNDC,							// Style Flags
		ClsWndProc::GetMyClassName(),
		pstrName,							// Name in Titelbar
		m_uiMyFlags,							// Flags wie das Window sich verhält
		m_pClsD3D11Recording->GetXPos(),	// Start X
		m_pClsD3D11Recording->GetYPos(),	// Start Y	
		m_pClsD3D11Recording->GetWndXSize(),// Width
		m_pClsD3D11Recording->GetWndYSize(),// Height
		NULL,								// ParentWindow
		NULL,								// Menu
		m_hInstance,						// Current hInstance 
		this								// CustomParameter, VoidPointer
	);
	CreateMyMenu();							// Simple Menu
	m_pClsD3D11Recording->SetHWND(m_hWnd);	// sets the WindowHandle in Superclass
}//END-CONSTR
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
	}//END-WHILE send MSg to ClsWndProc::MsgProcRun
	return TRUE;
}//END-RunMessageLoop
/// <summary>
/// returns the WindowHandle
/// CalledBy: not used yet
/// </summary>
/// <returns>WindowHandle</returns>
HWND ClsWnd::GetHWND() 
{ 
	return m_hWnd; 
}//END-FUNC
/// <summary>
/// Msg-Handling
/// CalledBy: ClsWndProc::MsgProcRun(...)/ClsWnd::RunMsgLoop()
/// - ClsWndProc::MsgProcRun(...) was set as CallbackFunction in ClsWndProc::MsgProcSetup
/// - This Function get called by ClsWndProc::MsgProcRun(...)
/// - ClsWndProc::MsgProcRun(...) get called by DispatchMessage in ClsWnd::RunMsgLoop()
/// </summary>
/// <param name="hWnd">Window Handle</param>
/// <param name="uiMsg">Window Message</param>
/// <param name="wParam">Additional message Info</param>
/// <param name="lParam">Additional message Info</param>
/// <returns></returns>
LRESULT ClsWnd::ProcessTriggeredMsg(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
	switch (uiMsg) 
	{
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
				DialogBox(
					m_hInstance,							// current Application Handle for the OS
					MAKEINTRESOURCE(IDD_ABOUTBOX),			// Msg
					m_hWnd,									// Window Handle
					&ClsWnd::About);						// Callback Function
				break;
			case IDM_EXIT:
				DestroyWindow(m_hWnd);
				break;
			default:
				return DefWindowProc(m_hWnd, uiMsg, wParam, lParam);
			}
		}
		break;
	}//END-SWITCH

	// Fallback default window proc
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}//END-FUNC ProcessTriggerMsg
/// <summary>
/// Loop this function until the About Dlg will be closed
/// </summary>
/// <param name="hDlg">Handle of the DlgBox</param>
/// <param name="message">Msg by User</param>
/// <param name="wParam">additional Parameter for the Msg</param>
/// <param name="lParam">additional Parameter for the Msg</param>
/// <returns></returns>
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
}//END-FUNC
/// <summary>
/// Creates a simple Menu.
/// - Needs a valid WindowHandle to identify the Window
/// </summary>
void ClsWnd::CreateMyMenu()
{
	ClsMenu oMyMenu(m_hWnd);
}//END-FUNC